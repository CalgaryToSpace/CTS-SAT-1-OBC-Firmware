import time

import serial

from tests_common import port, output

BAUD_RATE = 115200


def test_always_true():
    assert True


def test_port_construction(port):
    assert isinstance(port, serial.Serial)


def test_hello_world(port):
    with port as open_port:
        open_port.flush()
        open_port.write(b"CTS1+hello_world()!")
        open_port.flush()

        assert output(port, b"Hello, world!\n")

2

def test_echo_args(port):
    with port as open_port:
        open_port.flush() 
        open_port.write(b"CTS1+echo_back_args(hi)!")
        open_port.flush()
        assert output(open_port, b"SUCCESS: Echo Args: 'hi'\n")
