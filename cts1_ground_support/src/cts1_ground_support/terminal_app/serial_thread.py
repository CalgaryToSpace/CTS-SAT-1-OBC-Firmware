"""Serial thread for handling receiving UART communication."""

import threading
import time

import serial
from loguru import logger

from cts1_ground_support.terminal_app.app_config import MAX_RX_TX_LOG_ENTRIES, UART_BAUD_RATE
from cts1_ground_support.terminal_app.app_store import app_store
from cts1_ground_support.terminal_app.app_types import UART_PORT_NAME_DISCONNECTED, RxTxLogEntry


def uart_listener() -> None:
    """Listen for incoming UART data and log it to the app_store."""
    while True:
        if app_store.uart_port_name == UART_PORT_NAME_DISCONNECTED:
            time.sleep(0.1)
            continue

        try:
            with serial.Serial(app_store.uart_port_name, UART_BAUD_RATE, timeout=1) as port:
                while True:
                    if app_store.uart_port_name == UART_PORT_NAME_DISCONNECTED:
                        break

                    # Check for incoming data
                    if port.in_waiting > 0:
                        received_data: bytes = port.readline()
                        app_store.rxtx_log.append(RxTxLogEntry(received_data, "receive"))
                    elif len(app_store.rxtx_log) > MAX_RX_TX_LOG_ENTRIES:
                        # Data's not coming in rapid-fire, so take a sec to purge the buffer.
                        app_store.rxtx_log = app_store.rxtx_log[-MAX_RX_TX_LOG_ENTRIES:]

                    # Check for outgoing data
                    if len(app_store.tx_queue) > 0:
                        tx_data = app_store.tx_queue.pop(0)
                        port.write(tx_data)
                        app_store.rxtx_log.append(RxTxLogEntry(tx_data, "transmit"))
                        app_store.last_tx_timestamp_sec = time.time()

                    time.sleep(0.01)

        except serial.SerialException as e:
            msg = (
                f"Serial port forcefully disconnected (SerialException: {e}): "
                f"{app_store.uart_port_name}"
            )
            app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "error"))
            logger.warning(msg)
            app_store.uart_port_name = UART_PORT_NAME_DISCONNECTED  # propagate back to UI
            time.sleep(0.5)


def start_uart_listener() -> None:
    """Start the UART listener thread."""
    thread = threading.Thread(target=uart_listener)
    thread.daemon = True
    thread.start()
    logger.info("UART listener thread started.")
