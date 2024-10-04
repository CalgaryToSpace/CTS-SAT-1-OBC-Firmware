import pytest
import serial
import time
import os

# need to setup debugger in json file
# need to set up env variable
# need to properly unparse telecommand argument

@pytest.fixture
def config(): # config that establishes python connection to UART based off of env variables
    # env_var = os.environ["ENV_VAR"]
    ser = serial.Serial('COM3', 115200)
    return ser

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
   
    # print(var)
    config.write(b'CTS1+hello_world()!')
    check = output(config,'Hello, world!\n')
    config.close()
    assert check
    
def test_true():
    assert True



