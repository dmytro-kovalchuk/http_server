import pytest
import ctypes


def get_last_line():
    with open("log.txt", "r") as f:
        lines = f.readlines()
    return lines[-1].strip() if lines else ""

def get_level_name(num):
    match (num):
        case 0: return "DEBUG"
        case 1: return "INFO"
        case 2: return "WARN"
        case 3: return "ERROR"
        case 4: return "FATAL"
        case _: return "UNKNOWN"


@pytest.fixture
def logger_lib():
    lib = ctypes.CDLL("build/test_logger.so")

    lib.load_config.argtypes = [ctypes.c_char_p]
    lib.load_config.restype = None

    lib.log_message.argtypes = [ctypes.c_int, ctypes.c_char_p]
    lib.log_message.restype = None

    return lib


@pytest.fixture(autouse=True)
def auto_load_config(logger_lib):
    logger_lib.load_config("../config.json".encode())


@pytest.mark.parametrize(
        "level,message", 
        [
            (0, "Testing DEBUG"), 
            (1, "Testing INFO"), 
            (2, "Testing WARN"), 
            (3, "Testing ERROR"), 
            (4, "Testing FATAL"), 
            (999, "Testing UNKNOWN"),
            (0, ""),
            (0, "A"*1000)
        ]
)
def test_log(logger_lib, level, message):
    logger_lib.log_message(level, message.encode('utf-8'))

    line = get_last_line()
    
    assert message in line
    assert get_level_name(level) in line

def test_log_none_message(logger_lib):
    with open("log.txt", "w") as f:
        f.truncate(0)

    logger_lib.log_message(0, None)
    line = get_last_line()
    assert line == ""