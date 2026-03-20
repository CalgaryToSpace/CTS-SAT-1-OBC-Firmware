import hashlib
import re
import sys
from pathlib import Path


def extract_radio_packets_from_logs(packet_type: int, log_file_path: Path) -> list[bytes]:
    """Extracts radio packets from the log file.

    Args:
        packet_type: The number of packets to extract.
        log_file_path: Path to the log file.

    Returns:
        A list of bytes representing the extracted radio packets.
    """
    regex = re.compile(rf"C2 A2 8A 00 {packet_type:02X} ([0-9A-F][0-9A-F] ?)+", re.IGNORECASE)

    packets: list[bytes] = []

    with open(log_file_path, "r", encoding="utf-8") as log_file:
        for line in log_file:
            match = regex.search(line)
            if match:
                hex_string = match.group(0).replace(" ", "")
                packet_bytes = bytes.fromhex(hex_string)
                packets.append(packet_bytes)
    return packets


def reconstruct_bulk_downlinked_file(log_file_path: Path, output_file_path: Path) -> None:
    """Reconstructs a bulk downlinked file from the log file.

    Args:
        log_file_path: Path to the log file.
        output_file_path: Path to save the reconstructed file.
    """
    byte_offset_list: list[int] = []
    with open(output_file_path, "wb") as output_file:
        for packet in extract_radio_packets_from_logs(0x10, log_file_path):
            # Read the offset in the file from bytes 5,6,7,8.
            offset = int.from_bytes(packet[5:9], "little")
            byte_offset_list.append(offset)

            # Write the packet to the output file.
            output_file.write(packet[9:])

            if len(byte_offset_list) == 1:
                print(f"First packet data: offset_bytes={offset}, length_bytes={len(packet[9:])}")

    # Check if all packets are present
    if len(byte_offset_list) != len(set(byte_offset_list)):
        print("Warning: Duplicate packet byte offsets found.")

    if byte_offset_list != sorted(byte_offset_list):
        print("Warning: Packet byte offsets are not in order.")

    # TODO: Validate the entire file size.


def calculate_sha256(file_path: Path) -> str:
    """Calculates the SHA-256 hash of a file.

    Args:
        file_path: Path to the file.

    Returns:
        The SHA-256 hash of the file as a hexadecimal string.
    """
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for byte_block in iter(lambda: f.read(4096), b""):
            sha256_hash.update(byte_block)
    return sha256_hash.hexdigest()


def main() -> None:
    if len(sys.argv) != 3:
        print(
            "Usage: python extract_radio_packets_from_logs.py <log_file_path> <output_file_path>"
        )
        sys.exit(1)

    log_file_path = Path(sys.argv[1])
    output_file_path = Path(sys.argv[2])

    if not log_file_path.exists():
        print(f"Log file does not exist: {log_file_path}")
        sys.exit(1)

    reconstruct_bulk_downlinked_file(log_file_path, output_file_path)
    print(f"Reconstructed file saved to: {output_file_path}")

    print(f"Output file size: {output_file_path.stat().st_size:,} bytes")

    sha256_hash = calculate_sha256(output_file_path)
    print(f"SHA-256 hash: {sha256_hash}")

    if sha256_hash.lower() in log_file_path.read_text().lower():
        print("🟢🟢 SHA-256 hash matches a value in the log file! Very good! 🟢🟢")
    else:
        print("🟡 SHA-256 hash not found in the input log file. Maybe the command wasn't run?")


if __name__ == "__main__":
    main()
