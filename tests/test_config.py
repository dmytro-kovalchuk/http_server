import os
import ctypes
import pytest


class Config(ctypes.Structure):
    _fields_ = [
        ("ip", ctypes.c_uint32),
        ("port", ctypes.c_uint),
        ("max_clients", ctypes.c_uint),
        ("root_directory", ctypes.c_char * 256),
        ("log_file", ctypes.c_char * 256),
    ]


def int_to_ip(ip_int):
    return ".".join(str((ip_int >> (8 * i)) & 0xFF) for i in reversed(range(4)))


@pytest.fixture
def config_lib():
    lib = ctypes.CDLL(os.path.abspath("build/test_config.so"))
    lib.load_config.argtypes = [ctypes.c_char_p]
    lib.get_config.restype = ctypes.POINTER(Config)
    return lib


@pytest.fixture
def sample_config_str():
    return b'{ "ip": "10.0.0.5", "port": 9090, "max_clients": 12, "root_directory": "/data", "log_file": "server.log" }\0'


def test_load_and_getters(config_lib, tmp_path, sample_config_str):
    config_path = tmp_path / "conf.json"
    config_path.write_bytes(sample_config_str)

    config_lib.load_config(str(config_path).encode())

    cfg = config_lib.get_config().contents

    assert int_to_ip(cfg.ip) == "10.0.0.5"
    assert cfg.port == 9090
    assert cfg.max_clients == 12
    assert cfg.root_directory.decode() == "/data"
    assert cfg.log_file.decode() == "server.log"


def test_missing_fields_use_defaults(config_lib, tmp_path):
    partial_config = b'{ "port": 8080 }\0'
    config_path = tmp_path / "partial.json"
    config_path.write_bytes(partial_config)

    config_lib.load_config(str(config_path).encode())

    cfg = config_lib.get_config().contents

    assert cfg.ip == 2130706433 # 127.0.0.1
    assert cfg.port == 8080
    assert cfg.max_clients == 5
    assert cfg.root_directory.startswith(b"./storage")
    assert cfg.log_file.startswith(b"log.txt")


def test_invalid_ip_and_port(config_lib, tmp_path):
    invalid_config = b'{ "ip": "999.999.999.999", "port": 99999 }\0'
    config_path = tmp_path / "invalid.json"
    config_path.write_bytes(invalid_config)

    config_lib.load_config(str(config_path).encode())

    cfg = config_lib.get_config().contents

    assert cfg.ip == 2130706433 # 127.0.0.1
    assert cfg.port == 8080