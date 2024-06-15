"""A singleton class to store the app's state. Also, the instance of that class."""

import time
from dataclasses import dataclass, field

from cts1_ground_support.terminal_app.app_types import UART_PORT_NAME_DISCONNECTED, RxTxLogEntry


@dataclass
class AppStore:
    """A singleton class to store the app's state."""

    uart_port_name: str = UART_PORT_NAME_DISCONNECTED
    rxtx_log: list[RxTxLogEntry] = field(
        default_factory=lambda: [RxTxLogEntry(b"Start of Log", "notice")]
    )
    start_timestamp_sec: float = field(default_factory=time.time)
    tx_queue: list[bytes] = field(default_factory=list)

    selected_command_name: str | None = None


app_store = AppStore()
