import serial
import time
import struct
import random

random.seed(42)

# Setup the serial port
com_port = "COM7"  # Replace with the actual COM port
baud_rate = 230400  # Baud rate

# Function to create the data frame
def create_data_frame():
    # Initialize an empty byte array
    data = bytearray(160)

    # Sync bytes (0x0c, 0xff, 0xff, 0x0c)
    data[0] = 0x0c
    data[1] = 0xff
    data[2] = 0xff
    data[3] = 0x0c
    
    # Frame counter
    struct.pack_into('>H', data, 4, 69)  # Store frame counter at bytes 5 and 6
    
    # Board temperature
    data[6] = 0x11
    data[7] = 0x11

    # Firmware Version
    data[8] = 0x08

    #MPI Unit ID
    data[9] = 0x09

    # Director Status
    data[10] = 0x0a
    data[11] = 0x0b

    # Inner Dome Voltage Setting
    data[12] = 0x0c
    data[13] = 0x0d

    # SPIB
    data[14] = 0x0e

    # Inner Dome Scan Index
    data[15] = 0x0f

    # Faceplate Voltage Setting
    data[16] = 0x10
    data[17] = 0x11

    # Faceplate Voltage ADC Reading
    data[18] = 0x12
    data[19] = 0x13

    # Inner Dome Voltage ADC Reading
    data[20] = 0x12
    data[21] = 0x13

    # Image data with 135 bytes. 2 bytes per pixel
    for i in range(22, 158, 2):
        pixel_value = random.randint(0, 255)
        struct.pack_into('>H', data, i, pixel_value)
    
    # CRC (last 2 bytes)
    data[158] = 0xdd
    data[159] = 0xdd

    return data

# Main function to send data over COM port
def send_data():
    ser = serial.Serial(com_port, baud_rate, timeout=1)  # Open COM port
    
    bytes_sent = 0
    start_time = time.time()  # Record start time

    while True:
        # Create data frame to send
        data = create_data_frame()
        hex_data = ' '.join(f'{byte:02x}' for byte in data)
        # print(hex_data)

        # Send data frame over the COM port
        ser.write(data)
        bytes_sent += len(data)

        # If we do end up reading, the 32 frames in a second won't happen
        if ser.in_waiting > 0:
            
            # Read the data available from the buffer
            incoming_data = ser.read(ser.in_waiting)  # Read all available data
            print(f"Received data: \n{incoming_data.decode('utf-8', errors='replace')}")
        
        # Wait for 0.0262 second before sending the next frame (trying to match the rate of sending 32 frames in a second)
        # We should be sending 5120 bytes per second
        time.sleep(0.0262)

        if time.time() - start_time >= 1:
            # print(bytes_sent)
            start_time = time.time()
            bytes_sent = 0

# Run the script
if __name__ == "__main__":
    send_data()
