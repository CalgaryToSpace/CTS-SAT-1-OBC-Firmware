"""A singleton class to store the app's state. Also, the instance of that class."""

import time
from dataclasses import dataclass, field

from cts1_ground_support.terminal_app.app_types import UART_PORT_NAME_DISCONNECTED, RxTxLogEntry


@dataclass
class AppStore:
    """A singleton class to store the app's state (across all clients)."""

    uart_port_name: str = UART_PORT_NAME_DISCONNECTED
    rxtx_log: list[RxTxLogEntry] = field(
        default_factory=lambda: [RxTxLogEntry(b"Start of Log", "notice")]
    )
    server_start_timestamp_sec: float = field(default_factory=time.time)
    last_tx_timestamp_sec: float = 0
    tx_queue: list[bytes] = field(default_factory=list)

    uart_log_refresh_rate_ms: int = 500


app_store = AppStore()
