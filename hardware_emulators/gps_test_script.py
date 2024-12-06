import serial

def main():
    # Configure the serial port
    print(serial.__file__)
    uart_port = "COM5"  # Replace with your UART port (e.g., COM3 on Windows)
    baud_rate = 115200  # Set your baud rate

    # Text to send when data is received
    response_text =  """#BESTXYZA,COM1,0,55.0,FINESTEERING,1419,340033.000,02000040,d821,2724; SOL_COMPUTED,NARROW_INT,-1634531.5683,-3664618.0326,4942496.3270,0.0099,
                    0.0219,0.0115,SOL_COMPUTED,NARROW_INT,0.0011,-0.0049,-0.0001,0.0199,0.0439,0.0230,"AAAA",0.250,1.000,0.000,12,11,11,11,0,01,0,33*e9eafeca"""
    

    try:
        # Open the serial port
        with serial.Serial(uart_port, baud_rate, timeout=1) as ser:
            print(f"Listening on {uart_port} at {baud_rate} baud rate.")

            while True:
                # Check if data is available on RX
                if ser.in_waiting > 0:
                    received_data = ser.read(ser.in_waiting)  # Read all incoming data
                    print(f"Received: {received_data.decode('utf-8', errors='ignore')}")
                    
                    # Send the response text
                    ser.write(response_text.encode('utf-8'))
                    print(f"Sent: {response_text}")

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("\nExiting program.")

if __name__ == "__main__":
    main()
