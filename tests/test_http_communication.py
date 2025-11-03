import ctypes
import pytest
from enum import IntEnum


class Method(IntEnum):
    UNKNOWN = 0
    GET = 1
    POST = 2
    DELETE = 3


class Request(ctypes.Structure):
    _fields_ = [
        ("method", ctypes.c_int),
        ("path", ctypes.c_char * 512),
        ("version", ctypes.c_char * 32),
        ("headers", ctypes.c_char * 8192),
        ("body", ctypes.c_char_p),
        ("body_size", ctypes.c_ulong),
    ]


class Response(ctypes.Structure):
    _fields_ = [
        ("status", ctypes.c_char * 64),
        ("headers", ctypes.c_char * 256),
        ("body", ctypes.c_char_p),
        ("body_size", ctypes.c_ulong),
    ]


@pytest.fixture
def http_communication_lib():
    lib = ctypes.CDLL("build/test_http_communication.so")

    lib.load_config.argtypes = [ctypes.c_char_p]
    lib.load_config.restype = None

    lib.parse_request.argtypes = [ctypes.c_char_p]
    lib.parse_request.restype = Request

    lib.create_response.argtypes = [Request]
    lib.create_response.restype = ctypes.c_char_p

    lib.parse_content_length.argtypes = [ctypes.c_char_p]
    lib.parse_content_length.restype = ctypes.c_size_t

    lib.is_keep_alive.argtypes = [ctypes.c_char_p]
    lib.is_keep_alive.restype = ctypes.c_int

    return lib


@pytest.fixture(autouse=True)
def auto_load_config(http_communication_lib):
    http_communication_lib.load_config("../config.json".encode())


def test_parse_request(http_communication_lib):
    raw = b"GET /test.txt HTTP/1.1\r\nConnection: keep-alive\r\n\r\n"
    req = http_communication_lib.parse_request(raw)
    assert req.method == Method.GET
    assert req.path.decode() == "/test.txt"
    assert req.version.decode() == "HTTP/1.1"
    assert b"Connection: keep-alive" in req.headers


def test_create_response_get(http_communication_lib):
    raw = b"GET /file.txt HTTP/1.1\r\nConnection: close\r\n\r\n"
    req = http_communication_lib.parse_request(raw)
    resp_str = http_communication_lib.create_response(req)
    resp_py = ctypes.string_at(resp_str)
    assert b"HTTP/1.1 200 OK" in resp_py or b"HTTP/1.1 404 Not Found" in resp_py


def test_content_length(http_communication_lib):
    headers = b"Content-Length: 123\r\nConnection: keep-alive\r\n\r\n"
    cl = http_communication_lib.parse_content_length(headers)
    assert cl == 123


def test_keep_alive(http_communication_lib):
    headers = b"Connection: keep-alive\r\n\r\n"
    assert http_communication_lib.is_keep_alive(headers) == 1
    headers = b"Connection: close\r\n\r\n"
    assert http_communication_lib.is_keep_alive(headers) == 0
