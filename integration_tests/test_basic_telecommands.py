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


# def test_block_delay(port): # no longer works with how the current uart listnener works
#     with port as open_port:
#         arg = 2000
#         open_port.write(b"CTS1+demo_blocking_delay(" + str(arg).encode("utf-8") + b")!")
#         initial_time_ms = int(time.time() * 1000)
#         output(port, b"")
#         current_time_ms = int(time.time() * 1000)
#         difference = abs(current_time_ms - initial_time_ms)
#         if difference<100:
#             assert True
#         else:
#             assert False    
