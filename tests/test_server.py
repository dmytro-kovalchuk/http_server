import ctypes
import socket
import pytest

class SockAddrIn(ctypes.Structure):
    _fields_ = [
        ("sin_family", ctypes.c_ushort),
        ("sin_port", ctypes.c_ushort),
        ("sin_addr", ctypes.c_uint32),
        ("sin_zero", ctypes.c_char * 8),
    ]


@pytest.fixture
def server_lib():
    lib = ctypes.CDLL("build/test_server.so")

    lib.create_file_descriptor.restype = ctypes.c_int

    lib.make_port_reusable.argtypes = [ctypes.c_int]
    lib.make_port_reusable.restype = None

    lib.create_server_addr.restype = SockAddrIn

    lib.bind_addr_to_socket.argtypes = [ctypes.c_int, SockAddrIn]
    lib.bind_addr_to_socket.restype = None

    lib.start_listening.argtypes = [ctypes.c_int]
    lib.start_listening.restype = None

    lib.send_method_continue.argtypes = [ctypes.c_int]
    lib.send_method_continue.restype = None

    lib.send_method_other.argtypes = [ctypes.c_int]
    lib.send_method_other.restype = None

    return lib


def test_create_file_descriptor(server_lib):
    fd = server_lib.create_file_descriptor()
    assert fd > 0
    sock = socket.fromfd(fd, socket.AF_INET, socket.SOCK_STREAM)
    sock.close()


def test_make_port_reusable(server_lib):
    sock = socket.socket()
    server_lib.make_port_reusable(sock.fileno())
    sock.close()


def test_create_server_addr(server_lib):
    addr = server_lib.create_server_addr()
    assert addr.sin_family == socket.AF_INET
    assert addr.sin_port == socket.htons(8080)
    assert addr.sin_addr == 0x0100007F  # 127.0.0.1 in hex


def test_bind_addr_to_socket(server_lib):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    addr = SockAddrIn()
    addr.sin_family = socket.AF_INET
    addr.sin_port = socket.htons(0) 
    addr.sin_addr = 0x0100007F # 127.0.0.1 in hex
    server_lib.bind_addr_to_socket(sock.fileno(), addr)
    sock.close()


def test_start_listening(server_lib):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.bind(("127.0.0.1", 0))
    server_lib.start_listening(sock.fileno())
    sock.close()


def test_send_method_continue(server_lib):
    client, server = socket.socketpair()
    server_lib.send_method_continue(server.fileno())
    data = client.recv(1024)
    assert data == b"HTTP/1.1 100 Continue\r\n\r\n"
    client.close()
    server.close()


def test_send_method_other(server_lib):
    client, server = socket.socketpair()
    server_lib.send_method_other(server.fileno())
    data = client.recv(1024)
    assert b"405 Method Not Allowed" in data
    client.close()
    server.close()