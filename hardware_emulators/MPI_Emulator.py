import serial

def listen_and_respond(com_port, baudrate=230400):
     with serial.Serial(com_port, baudrate, timeout=1) as ser:
        print(f"Listening on {com_port} at {baudrate} baudrate.")
            
        # Read in incoming mpi telecommand
        while True:
            if ser.in_waiting > 0:
                incoming_cmd = ser.read(ser.in_waiting)
                print(f"Received:   {incoming_cmd.hex()}")
                
                # Append 1 to the telecommand data received to indicate a successful reception & echo back
                response = incoming_cmd + bytes([0x01])
                ser.write(response)
                print(f"Responded:  {response.hex()}")

if __name__ == "__main__":
    # Specify the COM port to listen on
    COM_PORT = input("Enter port name (eg: COM6): ")
    listen_and_respond(COM_PORT)
