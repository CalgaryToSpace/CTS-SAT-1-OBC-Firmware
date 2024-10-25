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
        # assert output(open_port, "Hello, world!\n")
        # time.sleep(0.5)
        # FIXME: This only works sometimes. Need to figure out how to make it work always.
        resp = open_port.read(10_000)
        if b"Hello, world!\n" not in resp:
            breakpoint()
        assert b"Hello, world!\n" in resp


def test_echo_args(port):
    with port as open_port:
        open_port.write(b"CTS1+echo_back_args(hi)!")
        assert output(open_port, "SUCCESS: Echo Args: 'hi'\n")


def test_block_delay(port):
    initial_time_ms = int(time.time() * 1000)
    arg = 2000
    port.write(b"CTS1+demo_blocking_delay(" + str(arg).encode("utf-8") + b")!")
    output(port, "")
    current_time_ms = int(time.time() * 1000)
    difference = current_time_ms - initial_time_ms
    port.close()
    if arg < difference and difference < (
        arg + 500
    ):  # sets a range of correctness due to overhead from reading in python
        assert True
    else:
        assert False
