"""Serial port utility functions."""

import serial.tools
import serial.tools.list_ports


def list_serial_ports() -> list[str]:
    """List available serial ports."""
    # Get a list of available serial ports
    available_ports = serial.tools.list_ports.comports()

    # Create a list of port names to display in the GUI
    return [port.device for port in available_ports]
