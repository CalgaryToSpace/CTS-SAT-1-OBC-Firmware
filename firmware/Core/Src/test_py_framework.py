import pytest
import serial
import time
import os
from dotenv import load_dotenv
from pathlib import Path

@pytest.fixture
def config(): # config that establishes python connection to UART based off of env variables
    load_dotenv()
    ser = serial.Serial(os.getenv("PORT"), int(os.environ["BAUDRATE"]))
    return ser

def test_always_true(config):
    assert True

def output(config,expected_response):
    response = ""
    for i in range(10):
        byte_data = config.readline()
        string_data = byte_data.decode('utf-8')
        response = ""
        for i in reversed(string_data):
            response = i + response
            if response == expected_response : 
                return True
    return False

def test_hello_world(config): 
    config.write(b'CTS1+hello_world()!')
    check = output(config,'Hello, world!\n')
    config.close()
    assert check
    
def test_echo_args(config):
    arg = b"hi"
    config.write(b'CTS1+echo_back_args('+arg+b')!')
    check = output(config,"SUCCESS: Echo Args: 'hi'\n")
    config.close()
    assert check

def test_block_delay(config):
    initial_time_ms = int(time.time() * 1000)
    arg = 2000
    config.write(b'CTS1+demo_blocking_delay(' + str(arg).encode('utf-8') + b')!')
    output(config,"")
    current_time_ms = int(time.time() * 1000)
    difference = current_time_ms - initial_time_ms
    config.close()
    if arg<difference and difference<(arg+500): # sets a range of correctness due to overhead from reading in python
        assert True
    else:
        assert False






