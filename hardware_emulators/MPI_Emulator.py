import serial

def listen_and_respond(com_port, baudrate=230400):
    try:
        with serial.Serial(com_port, baudrate, timeout=1) as ser:
            print(f"Listening on {com_port} at {baudrate} baudrate.")
            
            while True:
                if ser.in_waiting > 0:
                    incoming_cmd = ser.read(ser.in_waiting)
                    print(f"Received: {incoming_cmd.hex()}")
                    
                    response = incoming_cmd + bytes([0x01])
                    ser.write(response)
                    print(f"Responded: {response.hex()}")

    except serial.SerialException as e:
        print(f"Serial error: {e}")

if __name__ == "__main__":
    # Specify the COM port to listen on
    COM_PORT = 'COM6'  # Change to your COM port
    listen_and_respond(COM_PORT)
