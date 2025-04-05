import serial
import time

from loguru import logger

def listen_and_respond(com_port, baudrate=230400, receive_timeout=0.2):
    # Set a shorter serial timeout to avoid unnecessary delays
    with serial.Serial(com_port, baudrate, timeout=0.05) as ser:
        logger.info(f"Listening on {com_port} at {baudrate} baudrate.")
        
        buffer = bytearray()  # Buffer to hold incoming data
        last_receive_time = time.time()  # Time when the last byte was received
        
        while True:
            if ser.in_waiting > 0:
                # Read in available incoming data
                incoming_cmd = ser.read(ser.in_waiting)
                buffer.extend(incoming_cmd)
                last_receive_time = time.time()  # Update the last receive time
                logger.info(f"OBC -> MPI_COMPUTER: {incoming_cmd.hex()}")

            # Check if more than 200 milliseconds have passed since the last received data
            if len(buffer) > 0 and (time.time() - last_receive_time) > receive_timeout:
                # Transmission is considered complete
                # Now echo back the entire buffer at once, not in chunks
                response = buffer + bytes([0x01])  # Append the echo byte
                ser.write(response)
                ser.flush()  # Ensure the data is sent immediately
                logger.info(f"MPI_COMPUTER -> OBC:  {response.hex()}")

                # Clear buffer for the next transmission
                buffer.clear()

                # Ensure we respond within the total 300ms window
                time_elapsed = time.time() - last_receive_time
                if time_elapsed > 0.3:
                    logger.info("MPI_COMPUTER -> OBC: Warning: Response time exceeded 300ms!")

if __name__ == "__main__":
    # Specify the COM port to listen on
    COM_PORT = input("Enter port name (eg: COM6): ")
    listen_and_respond(COM_PORT)
