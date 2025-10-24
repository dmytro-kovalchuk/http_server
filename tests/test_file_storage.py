import pytest
import os
import socket
import ctypes
import tempfile

@pytest.fixture
def file_storage_lib():
    lib = ctypes.CDLL("build/test_file_storage.so")
    
    lib.send_file.argtypes = [ctypes.c_int, ctypes.c_char_p]
    lib.send_file.restype = ctypes.c_int
    
    lib.receive_file.argtypes = [ctypes.c_int, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
    lib.receive_file.restype = ctypes.c_int

    lib.delete_file.argtypes = [ctypes.c_char_p]
    lib.delete_file.restype = ctypes.c_int

    lib.is_file_exists.argtypes = [ctypes.c_char_p]
    lib.is_file_exists.restype = ctypes.c_int

    lib.get_file_size.argtypes = [ctypes.c_char_p]
    lib.get_file_size.restype = ctypes.c_size_t

    lib.set_file_location.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lib.set_file_location.restype = None

    return lib

@pytest.fixture
def socket_pair():
    server, client = socket.socketpair()
    yield server, client
    server.close()
    client.close()


def test_send_file_success(file_storage_lib, socket_pair):
    server, client = socket_pair
    sent_data = b"Testing send_file func"

    with tempfile.NamedTemporaryFile(delete=False) as file:
        file.write(sent_data)
        file.flush()
        filename = file.name

    result = file_storage_lib.send_file(client.fileno(), filename.encode())
    assert result == 0

    received_data = server.recv(len(sent_data))
    assert received_data == sent_data

    os.remove(filename)


def test_receive_file_success(file_storage_lib, socket_pair):
    server, client = socket_pair

    dir = "storage/"
    os.makedirs(dir, exist_ok=True)
    filename = "test.txt"
    sent_data = b"Testing receive_file func"
    
    client.sendall(sent_data)

    result = file_storage_lib.receive_file(server.fileno(), filename.encode("utf-8"), len(sent_data), None, 0)
    assert result == 0

    with open(dir + filename, "rb") as file:
        received_data = file.read()
    assert received_data == sent_data

    os.remove(dir + filename)


def test_send_file_missing(file_storage_lib, socket_pair):
    server, client = socket_pair
    result = file_storage_lib.send_file(client.fileno(), b"fail")
    assert result == -1


def test_send_file_closed_socket(file_storage_lib, socket_pair):
    with tempfile.NamedTemporaryFile(delete=False) as file:
        file.write(b"Testing send_file func")
        file.flush()
        filename = file.name

    server, client = socket.socketpair()
    client.close()

    result = file_storage_lib.send_file(client.fileno(), filename.encode())
    assert result == -1

    os.remove(filename)
    server.close()


def test_receive_file_closed_socket(file_storage_lib, socket_pair):
    server, client = socket.socketpair()
    server.close()

    dir = "storage/"
    os.makedirs(dir, exist_ok=True)
    filename = "test.txt"

    result = file_storage_lib.receive_file(server.fileno(), filename.encode("utf-8"), 256, None, 0)
    assert result == -1

    os.remove(dir + filename)


def test_delete_file_success(file_storage_lib):
    dir = "storage/"
    filename = "test.txt"
    
    with open(dir + filename, "w") as file:
        file.write("data")

    assert file_storage_lib.delete_file(filename.encode()) == 0
    assert not os.path.exists(dir + filename)


def test_delete_file_missing(file_storage_lib):
    assert file_storage_lib.delete_file(b"test.txt") != 0


def test_is_file_exists(file_storage_lib):
    dir = "storage/"
    filename = "test.txt"
    
    with open(dir + filename, "w") as file:
        file.write("data")

    assert file_storage_lib.is_file_exists(filename.encode()) == 1
    os.remove(dir + filename)
    

def test_is_file_exists_missing(file_storage_lib):
    assert file_storage_lib.is_file_exists("storage/nonexist.txt".encode()) == 0


def test_get_file_size(file_storage_lib):
    dir = "storage/"
    filename = "test.txt"
    
    with open(dir + filename, "w") as file:
        file.write("data")

    assert file_storage_lib.get_file_size(filename.encode()) == os.path.getsize((dir + filename).encode())
    os.remove(dir + filename)


def test_set_file_location(file_storage_lib):
    buffer = ctypes.create_string_buffer(512)
    filename = b"test.txt"

    file_storage_lib.set_file_location(buffer, filename)

    assert buffer.value.decode("utf-8") == "storage/test.txt"