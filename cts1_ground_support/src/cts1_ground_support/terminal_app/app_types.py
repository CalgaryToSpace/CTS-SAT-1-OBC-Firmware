"""Type definitions for the app."""

import time
from dataclasses import dataclass, field
from typing import Literal


@dataclass
class RxTxLogEntry:
    """A class to store an entry in the RX/TX log."""

    raw_bytes: bytes
    entry_type: Literal["send", "receive", "notice"]
    timestamp_sec: float = field(default_factory=lambda: time.time())

    @property
    def style(self: "RxTxLogEntry") -> dict:
        """Get the style for the log entry."""
        if self.entry_type == "send":
            return {"color": "cyan"}
        if self.entry_type == "notice":
            return {"color": "yellow"}
        if self.entry_type == "receive":
            return {"color": "#AAFFAA"}  # green
        if self.entry_type == "error":
            return {"color": "#FF6666"}

        msg = f"Invalid entry type: {self.entry_type}"
        raise ValueError(msg)

    @property
    def text(self: "RxTxLogEntry") -> str:
        """Get the text representation of the log entry."""
        if self.entry_type == "notice":
            return f"==================== {self.raw_bytes.decode()} ===================="

        # TODO: represent binary/unprintable characters as hex
        return self.raw_bytes.decode()
