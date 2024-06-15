"""A singleton class to store the app's state. Also, the instance of that class."""

import time
from dataclasses import dataclass, field

from cts1_ground_support.terminal_app.app_types import RxTxLogEntry


@dataclass
class AppStore:
    """A singleton class to store the app's state."""

    uart_port_name: str = "disconnected"
    rxtx_log: list[RxTxLogEntry] = field(default_factory=list)
    start_timestamp_sec: float = field(default_factory=time.time)
    tx_queue: list[bytes] = field(default_factory=list)


app_store = AppStore()
