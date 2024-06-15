"""The main screen GUI, which allows sending commands, and viewing the RX/TX log.

The main screen has the following components:
- Left Side: The list of commands, as a pick list.
- Left Side: Text fields to input each argument for the command.
- Right Side: An "RX/TX" log, which shows the most recent received and transmitted messages
(occupies the right 70% of the screen).
"""

import argparse
import functools

import dash
import dash_bootstrap_components as dbc
from dash import callback, dcc, html
from dash.dependencies import Input, Output, State
from loguru import logger

from cts1_ground_support.serial import list_serial_ports
from cts1_ground_support.telecommand_array_parser import parse_telecommand_list_from_repo
from cts1_ground_support.telecommand_types import TelecommandDefinition
from cts1_ground_support.terminal_app.app_store import app_store
from cts1_ground_support.terminal_app.app_types import UART_PORT_NAME_DISCONNECTED, RxTxLogEntry
from cts1_ground_support.terminal_app.serial_thread import start_uart_listener

UART_PORT_OPTION_LABEL_DISCONNECTED = "⛔ Disconnected ⛔"

# TODO: log UART comms to a file
# TODO: add a "Jump to Bottom" button, if scrolled up at all
# TODO: setup variable polling interval: poll frequently right after sending a command, then slow
# TODO: render the unit test check marks correct (instead of as hex), if we want


@functools.lru_cache  # cache forever is fine
def get_telecommand_list_from_repo_cached() -> list[TelecommandDefinition]:
    """Get the telecommand list from the repo, and cache the result."""
    return parse_telecommand_list_from_repo()


def get_telecommand_name_list() -> list[str]:
    """Get a list of telecommand names by reading the telecommands from the repo."""
    telecommands = get_telecommand_list_from_repo_cached()
    return [tcmd.name for tcmd in telecommands]


def get_telecommand_by_name(name: str) -> TelecommandDefinition:
    """Get a telecommand definition by name."""
    telecommands = get_telecommand_list_from_repo_cached()
    telecommand = next((tcmd for tcmd in telecommands if tcmd.name == name), None)
    if not telecommand:
        msg = f"Telecommand not found: {name}"
        raise ValueError(msg)
    return telecommand


def get_max_arguments_per_telecommand() -> int:
    """Get the maximum number of arguments for any telecommand."""
    telecommands = get_telecommand_list_from_repo_cached()
    return max(tcmd.number_of_args for tcmd in telecommands)


@callback(
    Output("argument-inputs", "children"),
    Input("telecommand-dropdown", "value"),
)
def update_argument_inputs(selected_command_name: str) -> list[html.Div]:
    """Generate the argument input fields based on the selected telecommand."""
    selected_tcmd = get_telecommand_by_name(selected_command_name)

    if selected_tcmd is None:
        app_store.selected_command_name = None
        return []

    app_store.selected_command_name = selected_command_name

    return [
        dbc.FormFloating(
            [
                dbc.Input(
                    type="text",
                    id=(this_id := f"arg-input-{arg_num}"),
                    placeholder=f"Argument {arg_num}",
                    disabled=(arg_num >= selected_tcmd.number_of_args),
                    style={"font-family": "monospace"},
                ),
                dbc.Label(f"Argument {arg_num}", html_for=this_id),
            ],
            className="mb-3",
            # Hide the argument input if it is not needed for the selected telecommand.
            style=({"display": "none"} if arg_num >= selected_tcmd.number_of_args else {}),
        )
        for arg_num in range(get_max_arguments_per_telecommand())
    ]


def handle_uart_port_change(uart_port_name: str) -> None:
    """Update the serial port name in the app store, if the port name changes."""
    last_uart_port_name = app_store.uart_port_name

    if uart_port_name != last_uart_port_name:
        if uart_port_name == UART_PORT_NAME_DISCONNECTED:
            logger.debug(
                f"Disconnect. Last port: {last_uart_port_name}. New port: {uart_port_name}."
            )
            msg = "Serial port disconnected."
        elif last_uart_port_name == UART_PORT_NAME_DISCONNECTED:
            logger.debug(f"Connect. Last port: {last_uart_port_name}. New port: {uart_port_name}.")
            msg = f"Serial port connected: {uart_port_name}"
        else:
            msg = f"Serial port changed from {last_uart_port_name} to {uart_port_name}"

        logger.info(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "notice"))

    app_store.uart_port_name = uart_port_name


@callback(
    Input("send-button", "n_clicks"),
    State("telecommand-dropdown", "value"),
    # TODO: maybe this could be cleanear with `Input/State("argument-inputs", "children")`, somehow
    *[
        State(f"arg-input-{arg_num}", "value")
        for arg_num in range(get_max_arguments_per_telecommand())
    ],
    prevent_initial_call=True,
)
def send_button_callback(
    n_clicks: int, selected_command_name: str, *every_arg_value: tuple[str]
) -> None:
    """Handle the send button click event by adding the command to the TX queue."""
    logger.info(f"Send button clicked ({n_clicks=})!")

    if selected_command_name is None:
        msg = "No command selected. Can't send a command!"
        logger.error(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "error"))
        return

    args = [
        every_arg_value[arg_num]
        for arg_num in range(get_telecommand_by_name(selected_command_name).number_of_args)
    ]
    if any(arg is None or arg == "" for arg in args):
        msg = f"Not all arguments are filled in. Can't run {selected_command_name}{args}!"
        logger.error(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "error"))
        return

    if app_store.uart_port_name == UART_PORT_NAME_DISCONNECTED:
        msg = "Can't send command when disconnected."
        logger.error(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "error"))
        return

    logger.info(f"Adding command to queue: {selected_command_name}{args}")

    command_str = f"CTS1+{selected_command_name}({','.join(args)})!"
    app_store.tx_queue.append(command_str.encode("ascii"))


@callback(
    Input("clear-log-button", "n_clicks"),
    prevent_initial_call=True,
)
def clear_log_button_callback(n_clicks: int) -> None:
    """Handle the "Clear Log" button click event by resetting the log."""
    logger.info(f"Clear Log button clicked ({n_clicks=})!")
    app_store.rxtx_log = [RxTxLogEntry(b"Log Reset", "notice")].copy()


@callback(
    Output("uart-port-dropdown", "options"),
    Input("uart-port-dropdown", "value"),
    Input("uart-port-dropdown-interval-component", "n_intervals"),
)
def update_uart_port_dropdown_options(uart_port_name: str, _n_intervals: int) -> list[str]:
    """Update the UART port dropdown with the available serial ports."""
    if uart_port_name is None:
        uart_port_name = UART_PORT_NAME_DISCONNECTED
    handle_uart_port_change(uart_port_name)

    # Re-render the dropdown with the updated list of serial ports.
    port_name_list = list_serial_ports()
    if app_store.uart_port_name not in ([*port_name_list, UART_PORT_NAME_DISCONNECTED]):
        msg = f"Serial port is no longer available in list of ports: {app_store.uart_port_name}"
        logger.warning(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "error"))
        app_store.uart_port_name = UART_PORT_NAME_DISCONNECTED

    if app_store.uart_port_name != uart_port_name:
        logger.debug("Would try to update the selected UART port value to 'DISCONNECTED'.")

    # NOTE: Don't try to update the dropdown options in the callback, as it will trigger the
    # callback again and infinitely toggle between connected and disconnected.
    return [
        {"label": UART_PORT_OPTION_LABEL_DISCONNECTED, "value": UART_PORT_NAME_DISCONNECTED}
    ] + [{"label": port, "value": port} for port in port_name_list]


def generate_rx_tx_log() -> html.Div:
    """Generate the RX/TX log, which shows the most recent received and transmitted messages."""
    return html.Div(
        [
            html.P(entry.text, style=entry.css_style | {"margin": "0", "line-height": "1.1"})
            for entry in (app_store.rxtx_log)
        ],
        id="rx-tx-log",
        className="p-3",
    )


# Should be the last callback in the file, as other callbacks modify the log.
@callback(
    Output("rx-tx-log-container", "children"),
    Input("uart-port-dropdown", "value"),
    Input("send-button", "n_clicks"),
    Input("clear-log-button", "n_clicks"),
    Input("uart-update-interval-component", "n_intervals"),
)
def update_uart_log_interval(
    _uart_port_name: str,
    _n_clicks_send: int,
    _n_clicks_clear_logs: int,
    _update_interval_count: int,
) -> html.Div:
    """Update the UART log at the specified interval."""
    return generate_rx_tx_log()


def generate_left_pane() -> list:
    """Make the left pane of the GUI, to be put inside a Col."""
    return [
        html.H1("CTS-SAT-1 Ground Support - Telecommand Terminal", className="text-center"),
        dbc.Row(
            [
                dbc.Label("Select a Serial Port:", html_for="uart-port-dropdown"),
                dcc.Dropdown(
                    id="uart-port-dropdown",
                    options=(
                        [
                            {
                                "label": UART_PORT_OPTION_LABEL_DISCONNECTED,
                                "value": UART_PORT_NAME_DISCONNECTED,
                            }
                        ]
                        + [{"label": port, "value": port} for port in list_serial_ports()]
                    ),
                    value=UART_PORT_NAME_DISCONNECTED,
                    className="mb-3",  # Add margin bottom to the dropdown
                ),
                dcc.Interval(
                    id="uart-port-dropdown-interval-component",
                    interval=2500,  # in milliseconds
                    n_intervals=0,
                ),
            ],
        ),
        dbc.Row(
            [
                dbc.Label("Select a Telecommand:", html_for="telecommand-dropdown"),
                dcc.Dropdown(
                    id="telecommand-dropdown",
                    options=[{"label": cmd, "value": cmd} for cmd in get_telecommand_name_list()],
                    value=get_telecommand_name_list()[0],
                    className="mb-3",  # Add margin bottom to the dropdown
                    style={"font-family": "monospace"},
                ),
            ],
        ),
        dbc.Row(
            [
                dbc.Button(
                    "Clear Log 🫗",
                    id="clear-log-button",
                    n_clicks=0,
                    className="m-1 px-3",
                    style={"width": "auto"},
                    color="danger",
                ),
                dbc.Button(
                    "Send 📡",
                    id="send-button",
                    n_clicks=0,
                    className="m-1 px-5",
                    style={"width": "auto"},
                ),
            ],
            justify="center",
            className="mb-3",
        ),
        html.Div(id="argument-inputs", className="mb-3"),
    ]


def run_dash_app(*, enable_debug: bool = False) -> None:
    """Run the main Dash application."""
    app = dash.Dash(
        "CTS-SAT-1 Ground Support",
        external_stylesheets=[dbc.themes.BOOTSTRAP],
    )

    app.layout = dbc.Container(
        [
            dbc.Row(
                [
                    dbc.Col(
                        generate_left_pane(),
                        width=3,
                        style={"height": "100vh", "overflow-y": "scroll"},
                        class_name="p-3",
                    ),
                    dbc.Col(
                        [
                            html.Div(
                                generate_rx_tx_log(),
                                id="rx-tx-log-container",
                                style={
                                    "font-family": "monospace",
                                    "background-color": "black",
                                    "height": "100%",
                                    "overflowY": "auto",
                                    "flex-direction": "column-reverse",
                                    "display": "flex",
                                },
                            ),
                            dcc.Interval(
                                id="uart-update-interval-component",
                                interval=1000,  # in milliseconds
                                n_intervals=0,
                            ),
                        ],
                        width=9,
                        style={"height": "100vh", "overflow-y": "scroll"},
                    ),
                ],
            ),
        ],
        fluid=True,  # Use a fluid container for full width
    )

    start_uart_listener()

    app.run_server(debug=enable_debug)
    logger.info("Dash app started and finished.")


def main() -> None:
    """Run the main server, with optional debug mode (via CLI arg)."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-d",
        "--debug",
        action="store_true",
        help="Enable debug mode for the Dash app.",
    )
    args = parser.parse_args()
    run_dash_app(enable_debug=args.debug)


if __name__ == "__main__":
    main()
