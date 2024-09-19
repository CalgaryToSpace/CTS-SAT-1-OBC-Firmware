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

def test_hello_world(config): 
    config.write(b'CTS1+hello_world()!')
    config.close()
    assert True
    
def test_true():
    assert True

def test_false():
    assert False



