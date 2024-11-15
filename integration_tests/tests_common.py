import os

import pytest
import serial
import time
from dotenv import load_dotenv

BAUD_RATE = 115200


@pytest.fixture
def port() -> serial.Serial:
    """Port that establishes python connection to UART based off of env variables.

    Returns:
        Serial object, connected to the UART port of the CTS-SAT-1 OBC/dev kit.
    """
    load_dotenv()
    port_name = os.environ["PY_TEST_PORT"]
    assert port_name is not None

    # Important note: Timeout is in seconds!
    ser = serial.Serial(port_name, baudrate=BAUD_RATE, timeout=0.5)
    return ser


def output(port, expected_response: bytes) -> bool:
    start_time = time.time()
    everything: bytes = b'' # currently for debugging purposes 
    timeout = 4 # in seconds
    while True:
        if port.in_waiting>0:
            resp: bytes = port.readline() 
            everything = everything + resp # contains all output of uart terminal
            if expected_response in resp:
                return expected_response in resp
        if time.time() - start_time > timeout: # timeout for 4 seconds if it is unable to read the line
            break # comment out timeout if testing in debug mode
    return False
