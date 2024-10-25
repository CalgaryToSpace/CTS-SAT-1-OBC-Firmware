import os

import pytest
import serial
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


def output(port, expected_response: str) -> bool:
    response = ""
    for i in range(10):
        byte_data = port.readline()
        string_data = byte_data.decode("utf-8")
        response = ""
        for i in reversed(string_data):
            response = i + response
            if response == expected_response:
                return True
    return False
