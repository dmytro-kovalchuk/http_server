import os
import ctypes
import tempfile
import pytest


class Config(ctypes.Structure):
    _fields_ = [
        ("ip", ctypes.c_char * 32),
        ("port", ctypes.c_uint),
        ("max_clients", ctypes.c_uint),
        ("root_directory", ctypes.c_char * 256),
        ("log_file", ctypes.c_char * 256),
    ]


@pytest.fixture
def config_lib():
    lib = ctypes.CDLL(os.path.abspath("build/test_config.so"))

    lib.parse_config.argtypes = [ctypes.c_char_p]
    lib.parse_config.restype = Config

    lib.read_config.argtypes = [ctypes.c_char_p]
    lib.read_config.restype = ctypes.c_char_p

    lib.get_value_from_config.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p]
    lib.get_value_from_config.restype = ctypes.c_int

    lib.load_config.argtypes = [ctypes.c_char_p]

    lib.get_ip_from_config.restype = ctypes.c_char_p
    
    lib.get_root_dir_from_config.restype = ctypes.c_char_p
    
    lib.get_log_file_from_config.restype = ctypes.c_char_p
    
    lib.get_port_from_config.restype = ctypes.c_uint
    
    lib.get_max_clients_from_config.restype = ctypes.c_uint

    return lib


@pytest.fixture
def sample_config_str():
    return b'{ "ip": "10.0.0.5", "port": 9090, "max_clients": 12, "root_directory": "/data", "log_file": "server.log" }\0'


def test_parse_config_defaults(config_lib):
    config = config_lib.parse_config(None)
    assert config.ip.decode() == "127.0.0.1"
    assert config.port == 8080
    assert config.max_clients == 5
    assert config.root_directory.decode() == "./storage"
    assert config.log_file.decode() == "log.txt"


def test_parse_config_custom(config_lib, sample_config_str):
    config = config_lib.parse_config(sample_config_str)
    assert config.ip.decode() == "10.0.0.5"
    assert config.port == 9090
    assert config.max_clients == 12
    assert config.root_directory.decode() == "/data"
    assert config.log_file.decode() == "server.log"


def test_read_config(config_lib, tmp_path):
    config_path = tmp_path / "config.json"
    config_content = b'{ "ip": "1.2.3.4" }'
    config_path.write_bytes(config_content)

    result = config_lib.read_config(str(config_path).encode())
    assert config_content in result


def test_get_value_from_config(config_lib, sample_config_str):
    output = ctypes.create_string_buffer(256)
    ret = config_lib.get_value_from_config(sample_config_str, b"ip", output)
    assert ret == 0
    assert output.value.decode() == "10.0.0.5"


def test_load_and_getters(config_lib, tmp_path, sample_config_str):
    config_path = tmp_path / "conf.json"
    config_path.write_bytes(sample_config_str)

    config_lib.load_config(str(config_path).encode())

    assert config_lib.get_ip_from_config().decode() == "10.0.0.5"
    assert config_lib.get_port_from_config() == 9090
    assert config_lib.get_max_clients_from_config() == 12
    assert config_lib.get_root_dir_from_config().decode() == "/data"
    assert config_lib.get_log_file_from_config().decode() == "server.log"