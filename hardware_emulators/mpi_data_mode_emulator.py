"""Run this script before sending "enable_active_mode" command.

Data will only start transmitting once this script receives {0x54, 0x43, 0x13}.

Run with:

```bash
uv run hardware_emulators/mpi_data_mode_emulator.py COM7
```

"""

# /// script
# dependencies = [
#   "pyserial==3.5",
#   "loguru",
# ]
# ///

import time
import struct
import random
import sys

import serial
from loguru import logger

random.seed(42)


baud_rate = 230400  # Baud rate


def create_data_frame(frame_seq_num: int) -> bytearray:
    """Create a single MPI data frame with dummy data."""
    # Initialize an empty byte array
    data = bytearray(160)

    # Sync bytes (0x0c, 0xff, 0xff, 0x0c)
    data[0] = 0x0C
    data[1] = 0xFF
    data[2] = 0xFF
    data[3] = 0x0C

    # Frame counter
    struct.pack_into(">H", data, 4, frame_seq_num)  # Store frame counter at bytes 5 and 6

    # Board temperature
    data[6] = 0x11
    data[7] = 0x11

    # Firmware Version
    data[8] = 0x08

    # MPI Unit ID
    data[9] = 0x09

    # Director Status
    data[10] = 0x0A
    data[11] = 0x0B

    # Inner Dome Voltage Setting
    data[12] = 0x0C
    data[13] = 0x0D

    # SPIB
    data[14] = 0x0E

    # Inner Dome Scan Index
    data[15] = 0x0F

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
        struct.pack_into(">H", data, i, pixel_value)

    # CRC (last 2 bytes)
    data[158] = 0xDD
    data[159] = 0xDD

    return data


# Main function to send data over COM port
def send_data(com_port: str) -> None:
    is_mpi_active = False

    ser = serial.Serial(com_port, baud_rate, timeout=1)  # Open COM port

    total_bytes_sent = 0
    total_frames_sent = 0
    start_time = time.time()  # Record start time
    last_log_time = start_time

    sleep_between_frames = 0.0  # Gets tweaked to achieve approximately the target rate.
    target_frame_rate_per_second = 32.0

    while True:
        # Send data frame over the COM port.
        if is_mpi_active is False:
            if ser.in_waiting > 0:
                # Read the data available from the buffer (only if inactive - too slow otherwise).
                incoming_data = ser.read(ser.in_waiting)  # Read all available data
                received_hex = " ".join([f"{byte:02X}" for byte in incoming_data])

                if "54 43 13" in received_hex:
                    is_mpi_active = True
                    logger.info("MPI ON now. Starting data transmission.")
                elif "54 43 14" in received_hex:  # Note: This branch is never used/run.
                    is_mpi_active = False
                    logger.info("MPI OFF now. Stopping data transmission.")

        else:  # MPI is active.
            # Create data frame to send.
            data = create_data_frame(total_frames_sent)

            ser.write(data)
            total_bytes_sent += len(data)
            total_frames_sent += 1

            # Adjust sleep time to maintain target frame rate.
            time.sleep(sleep_between_frames)

            # Log every 5 seconds, or on the very first send.
            if time.time() - last_log_time >= 5 or (total_frames_sent == 1):
                hex_data = " ".join(f"{byte:02x}" for byte in data)
                logger.info(f"Sample data: {hex_data}")

                elapsed_time = time.time() - start_time
                avg_bytes_per_second = total_bytes_sent / elapsed_time
                avg_frames_per_second = total_frames_sent / elapsed_time

                logger.info(
                    f"Total sent: {total_bytes_sent:,} bytes = {total_frames_sent:,} frames. "
                    f"Time elapsed: {elapsed_time:.2f} seconds. "
                    f"Average rate: {avg_bytes_per_second:.2f} bytes/second = {avg_frames_per_second:.2f} frames/second."
                )
                last_log_time = time.time()

                # Recalculate sleep time to maintain target frame rate.
                sleep_between_frames = max(
                    0.0,
                    (1.0 / target_frame_rate_per_second) - (1.0 / avg_frames_per_second),
                )
                logger.debug(
                    f"Adjusted sleep time between frames to {sleep_between_frames:.6f} seconds."
                )


# Run the script
if __name__ == "__main__":
    send_data(com_port=sys.argv[1])
