"""Type definitions for the app."""

import time
from dataclasses import dataclass, field
from datetime import datetime
from typing import Literal

import pytz

from cts1_ground_support.bytes import bytes_to_nice_str

UART_PORT_NAME_DISCONNECTED = "disconnected"


@dataclass
class RxTxLogEntry:
    """A class to store an entry in the RX/TX log."""

    raw_bytes: bytes
    entry_type: Literal["transmit", "receive", "notice", "error"]
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

    def to_string(
        self: "RxTxLogEntry", *, show_end_of_line_chars: bool, show_timestamp: bool
    ) -> str:
        """Get the text representation of the log entry."""
        prefix = ""
        if show_timestamp:
            dt = datetime.fromtimestamp(self.timestamp_sec, tz=pytz.timezone("America/Edmonton"))
            # Format the datetime object to include milliseconds
            prefix = dt.strftime("%Y-%m-%d %H:%M:%S.%f")[:-3] + ": "

        if self.entry_type == "notice":
            return f"{prefix}==================== {self.raw_bytes.decode()} ===================="
        # TODO: make these equals-signs a fixed width
        if self.entry_type == "error":
            return f"{prefix}==================== {self.raw_bytes.decode()} ===================="

        return f"{prefix}" + bytes_to_nice_str(
            self.raw_bytes, show_end_of_line_chars=show_end_of_line_chars
        )
