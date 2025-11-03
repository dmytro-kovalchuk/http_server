import os
import ctypes
import tempfile
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

    lib.get_ip_from_config.restype = ctypes.c_uint32
    lib.get_port_from_config.restype = ctypes.c_uint
    lib.get_max_clients_from_config.restype = ctypes.c_uint
    lib.get_root_dir_from_config.restype = ctypes.c_char_p
    lib.get_log_file_from_config.restype = ctypes.c_char_p

    return lib


@pytest.fixture
def sample_config_str():
    return b'{ "ip": "10.0.0.5", "port": 9090, "max_clients": 12, "root_directory": "/data", "log_file": "server.log" }\0'


def test_load_and_getters(config_lib, tmp_path, sample_config_str):
    config_path = tmp_path / "conf.json"
    config_path.write_bytes(sample_config_str)

    config_lib.load_config(str(config_path).encode())

    ip_int = config_lib.get_ip_from_config()
    assert int_to_ip(ip_int) == "10.0.0.5"

    assert config_lib.get_port_from_config() == 9090
    assert config_lib.get_max_clients_from_config() == 12
    assert config_lib.get_root_dir_from_config().decode() == "/data"
    assert config_lib.get_log_file_from_config().decode() == "server.log"