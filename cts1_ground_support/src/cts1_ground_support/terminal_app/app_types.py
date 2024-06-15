"""Type definitions for the app."""

import time
from dataclasses import dataclass, field
from typing import Literal

from cts1_ground_support.bytes import bytes_to_nice_str

UART_PORT_NAME_DISCONNECTED = "disconnected"


@dataclass
class RxTxLogEntry:
    """A class to store an entry in the RX/TX log."""

    raw_bytes: bytes
    entry_type: Literal["transmit", "receive", "notice"]
    timestamp_sec: float = field(default_factory=lambda: time.time())

    @property
    def css_style(self: "RxTxLogEntry") -> dict:
        """Get the CSS style for the log entry (mostly just color currently)."""
        if self.entry_type == "transmit":
            return {"color": "cyan"}
        if self.entry_type == "receive":
            return {"color": "#AAFFAA"}  # green

        if self.entry_type == "notice":
            return {"color": "yellow"}

        if self.entry_type == "error":
            return {"color": "#FF6666"}

        msg = f"Invalid entry type: {self.entry_type}"
        raise ValueError(msg)

    @property
    def text(self: "RxTxLogEntry") -> str:
        """Get the text representation of the log entry."""
        if self.entry_type == "notice":
            return f"==================== {self.raw_bytes.decode()} ===================="

        # TODO: include timestamp as arg
        # TODO: specific linefeed/carriage return symbol
        return bytes_to_nice_str(self.raw_bytes)
