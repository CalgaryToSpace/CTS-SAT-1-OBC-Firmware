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
    packet_seq_nums = []
    with open(output_file_path, "wb") as output_file:
        for packet in extract_radio_packets_from_logs(0x10, log_file_path):
            packet_seq_num = packet[5]
            packet_max_seq_num = packet[6]

            packet_seq_nums.append(packet_seq_num)

            output_file.write(packet[11:])

    # Check if all packets are present
    if len(packet_seq_nums) != len(set(packet_seq_nums)):
        print("Warning: Duplicate packet sequence numbers found.")

    if len(packet_seq_nums) != packet_max_seq_num:
        print("Warning: Not all packets were received. Expected sequence numbers do not match.")
        print(f"Expected: {packet_max_seq_num}, Received: {len(packet_seq_nums)}")

    if packet_seq_num != packet_max_seq_num:
        print(
            "Warning: Last packet sequence number does not match the expected maximum sequence number."
        )


if __name__ == "__main__":
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
