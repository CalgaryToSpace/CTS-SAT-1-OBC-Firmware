"""Bulk uplink a file to a local devkit with telecommands.

Run with:

```bash
uv run misc_tools/bulk_uplink_to_devkit.py ./firmware/build/debug/CTS-SAT-1_FW.bin -o firmware.elf --port <uart_port>

# Current fastest working settings on UART yields 357 B/s:
--chunk-size 150 --delay 0.4

# ELF: 2.23MB file at that speed takes 1h45m (which is 11x 10-minute passes). <2 weeks.
# BIN: 277kB file at that speed takes 13 minutes (only 2 passes). Very good.
```

"""

# /// script
# dependencies = [
#   "loguru",
#   "pyserial",
#   "tqdm",
# ]
# ///

import argparse
import base64
import hashlib
import serial
import time
import sys
from pathlib import Path
import re

from loguru import logger
from tqdm import tqdm


def _send_simple_slow_command(ser: serial.Serial, command: str) -> bytes:
    ser.write(command.encode("ascii"))
    logger.debug(f"Sending: {command}")
    time.sleep(1)
    response_bytes = ser.read(ser.in_waiting)

    try:
        logger.debug(f"Response: {response_bytes.decode()}")
    except UnicodeDecodeError:
        logger.debug("Response: " + str(response_bytes))

    time.sleep(0.25)

    return response_bytes


def send_file_over_uart(
    input_file_path: Path,
    *,
    output_file: str,
    uart_port: str,
    baudrate: int,
    chunk_size: int,
    delay: float,
) -> None:
    # Open serial port
    with serial.Serial(
        port=uart_port,
        baudrate=baudrate,
        timeout=1,
        write_timeout=1,
    ) as ser:
        logger.info(f"Opened UART {uart_port} @ {baudrate} baud")

        time.sleep(1)
        logger.debug(f"Purging incoming serial buffer: {ser.read(ser.in_waiting)}")

        _send_simple_slow_command(ser, "CTS1+comms_bulk_uplink_close_file()!")

        _send_simple_slow_command(
            ser, f"CTS1+comms_bulk_uplink_open_file({output_file},truncate)!"
        )

        # Find ways to increase execution speed.
        _send_simple_slow_command(
            ser, "CTS1+config_set_int_var(AX100_enable_downlink_inhibited_uart_logs,0)!"
        )
        _send_simple_slow_command(ser, "CTS1+config_set_int_var(TASK_heartbeat_period_ms,0)!")
        _send_simple_slow_command(ser, "CTS1+log_set_sink_enabled_state(4,0)!")  # Disable UART.

        with (
            input_file_path.open("rb") as f,
            tqdm(total=input_file_path.stat().st_size, unit="B", unit_scale=True) as progress_bar,
        ):
            logger.info(
                f"Starting file uplink: {input_file_path} to {output_file} "
                f"({input_file_path.stat().st_size:,} bytes)"
            )
            chunk_index = 0
            problematic_chunk_indexes: list[int] = []
            while True:
                chunk = f.read(chunk_size)
                if not chunk:
                    logger.info("End of file reached")
                    break

                # Base64 encode the chunk
                b64_data = base64.b64encode(chunk).decode("ascii")

                # Construct message
                message = f"CTS1+bulkup64({b64_data})!"

                # Send over UART (add newline if protocol expects it)
                ser.write(message.encode("ascii"))
                logger.debug(f"Sent chunk {chunk_index}: {message}")

                progress_bar.update(len(chunk))

                if delay > 0:
                    time.sleep(delay)

                incoming_bytes = ser.read(ser.in_waiting)
                logger.debug(f"Resp: {incoming_bytes.decode()}")

                # Expected message when logs enabled.
                expected_str_1 = r"Telecommand 'bulkup64' executed. Duration=\dms, err=0"
                _expected_str_1_matches = re.findall(
                    expected_str_1, incoming_bytes.decode("ascii", errors="ignore")
                )

                # Expected message when logs disabled.
                expected_str_2 = (
                    f"==========================\n{len(chunk)}\n=========================="
                )
                # if (len(expected_str_1_matches) != 1):
                if incoming_bytes.decode("ascii", errors="ignore").count(expected_str_2) != 1:
                    problematic_chunk_indexes.append(chunk_index)
                    logger.warning(
                        f"Unexpected response from device during bulk uplink. "
                        f"Problematic chunks ({len(problematic_chunk_indexes)}): "
                        f"{problematic_chunk_indexes[-100:]}"
                    )

                chunk_index += 1

        _send_simple_slow_command(ser, "CTS1+comms_bulk_uplink_close_file()!")

        hash_on_satellite = _send_simple_slow_command(
            ser, f"CTS1+fs_read_file_sha256_hash_json({output_file},0,0)!"
        )

        hash_on_disk = hashlib.sha256(input_file_path.read_bytes()).hexdigest()
        logger.info(f"SHA256 hash of input file (computer-side): {hash_on_disk}")

        if hash_on_disk.lower().encode() in hash_on_satellite.lower():
            logger.success("SHA256 hash of input file matches hash on satellite.")
        else:
            logger.error("SHA256 hash of input file does NOT match hash on satellite.")


def main():
    parser = argparse.ArgumentParser(
        description="Send a file over UART using CTS1+bulkup64 messages"
    )
    parser.add_argument(
        "file",
        type=Path,
        help="Path to file to send",
    )
    parser.add_argument(
        "-p",
        "--port",
        required=True,
        help="UART port (e.g. /dev/ttyUSB0 or COM3)",
    )
    parser.add_argument(
        "-o",
        "--output-file",
        type=str,
        help="Destination filename on satellite filesystem.",
    )
    parser.add_argument(
        "--baudrate",
        type=int,
        default=115200,
        help="UART baudrate (default: 115200)",
    )
    parser.add_argument(
        "--chunk-size",
        type=int,
        default=64,
        help="Chunk size in bytes before base64 encoding (default: 64)",
    )
    parser.add_argument(
        "--delay",
        type=float,
        default=0.25,
        help="Delay in seconds between chunks (default: 0.25)",
    )

    args = parser.parse_args()

    input_file_path = Path(args.file)

    if not input_file_path.exists():
        logger.error(f"File not found: {input_file_path}")
        sys.exit(1)

    send_file_over_uart(
        input_file_path=input_file_path,
        output_file=args.output_file,
        uart_port=args.port,
        baudrate=int(args.baudrate),
        chunk_size=int(args.chunk_size),
        delay=float(args.delay),
    )


if __name__ == "__main__":
    main()
