"""The main screen GUI, which allows sending commands, and viewing the RX/TX log.

The main screen has the following components:
- Left Side: The list of commands, as a pick list.
- Left Side: Text fields to input each argument for the command.
- Right Side: An "RX/TX" log, which shows the most recent received and transmitted messages
(occupies the right 70% of the screen).
"""

import dash
import dash_bootstrap_components as dbc
from dash import callback, dcc, html
from dash.dependencies import Input, Output
from loguru import logger

from cts1_ground_support.serial import list_serial_ports
from cts1_ground_support.telecommand_array_parser import parse_telecommand_list_from_repo
from cts1_ground_support.telecommand_types import TelecommandDefinition
from cts1_ground_support.terminal_app.app_store import app_store
from cts1_ground_support.terminal_app.app_types import RxTxLogEntry
from cts1_ground_support.terminal_app.serial_thread import start_uart_listener


def get_telecommand_name_list() -> list[str]:
    """Get a list of telecommand names by reading the telecommands from the repo."""
    telecommands = parse_telecommand_list_from_repo()
    return [tcmd.name for tcmd in telecommands]


def get_telecommand_by_name(name: str) -> TelecommandDefinition:
    """Get a telecommand definition by name."""
    telecommands = parse_telecommand_list_from_repo()
    telecommand = next((tcmd for tcmd in telecommands if tcmd.name == name), None)
    if not telecommand:
        msg = f"Telecommand not found: {name}"
        raise ValueError(msg)
    return telecommand


@callback(Output("argument-inputs", "children"), Input("telecommand-dropdown", "value"))
def update_argument_inputs(selected_command_name: str) -> list[html.Div]:
    """Generate the argument input fields based on the selected telecommand."""
    selected_tcmd = get_telecommand_by_name(selected_command_name)

    if selected_tcmd is None:
        return []

    return [
        dbc.FormFloating(
            [
                dbc.Input(
                    type="text",
                    id=(this_id := f"arg-input-{selected_command_name}-{arg_num}"),
                    placeholder=f"Argument {arg_num}",
                ),
                dbc.Label(f"Argument {arg_num}", html_for=this_id),
            ],
            className="mb-3",
        )
        for arg_num in range(selected_tcmd.number_of_args)
    ]


def handle_uart_port_change(uart_port_name: str) -> None:
    """Update the serial port name in the app store, if the port name changes."""
    last_uart_port_name = app_store.uart_port_name

    if uart_port_name != last_uart_port_name:
        if uart_port_name == "disconnected":
            msg = "Serial port disconnected."
        elif last_uart_port_name == "disconnected":
            msg = f"Serial port connected: {uart_port_name}"
        else:
            msg = f"Serial port changed from {last_uart_port_name} to {uart_port_name}"

        logger.info(msg)
        app_store.rxtx_log.append(RxTxLogEntry(msg.encode(), "notice"))

    app_store.uart_port_name = uart_port_name


@callback(
    Input("send-button", "n_clicks"),
    Input("telecommand-dropdown", "value"),
    prevent_initial_call=True,
)
def send_button_callback(n_clicks: int, selected_command_name: str) -> None:
    """Handle the send button click event by adding the command to the TX queue."""
    logger.info(f"Send button clicked {n_clicks=} times!")
    args = [app_store[dcc.Input(f"arg-input-{selected_command_name}-{arg_num}").value] for arg_num in range(get_telecommand_by_name(selected_command_name).number_of_args)]
    command = f"{selected_command_name},{','.join(args)}".encode()
    app_store.tx_queue.append(command)


@callback(
    Output("rx-tx-log", "children"),
    Input("uart-port-dropdown", "value"),
    Input("send-button", "n_clicks"),
    Input("uart-update-interval-component", "n_intervals"),
)
def update_uart_log_interval(
    uart_port_name: str,
    n_clicks_send: int,
    update_interval_count: int,
) -> html.Div:
    """Update the UART log at the specified interval."""
    # Checks if the UART port status changed, record it appropriately in the app_store.
    handle_uart_port_change(uart_port_name)

    return generate_rx_tx_log()


def generate_rx_tx_log() -> html.Div:
    """Generate the RX/TX log, which shows the most recent received and transmitted messages."""
    return html.Div(
        [html.P(entry.text, style=entry.style) for entry in app_store.rxtx_log],
        style={
            "font-family": "monospace",
            "background-color": "black",
            "height": "100%",
            "overflowY": "auto",
        },
        id="rx-tx-log",
    )


@callback(
    Output("uart-port-dropdown", "options"),
    Output("uart-port-dropdown", "value"),
    Input("uart-port-dropdown-interval-component", "n_intervals"),
)
def update_uart_port_dropdown(_n_intervals: int) -> dcc.Dropdown:
    """Update the UART port dropdown with the available serial ports."""
    port_name_list = list_serial_ports()
    if app_store.uart_port_name not in ([*port_name_list, "disconnected"]):
        logger.warning(
            f"Force disconnecting from serial port in callback: {app_store.uart_port_name}"
        )
        app_store.uart_port_name = "disconnected"

    return (
        (
            [{"label": "⛔ Disconnected ⛔", "value": "disconnected"}]
            + [{"label": port, "value": port} for port in port_name_list]
        ),
        app_store.uart_port_name,
    )


def generate_left_pane() -> list:
    """Make the left pane of the GUI, to be put inside a col."""
    return [
        dbc.Row(
            [
                dbc.Label("Select a Serial Port:", html_for="uart-port-dropdown"),
                dcc.Dropdown(
                    id="uart-port-dropdown",
                    options=(
                        [{"label": "⛔ Disconnected ⛔", "value": "disconnected"}]
                        + [{"label": port, "value": port} for port in list_serial_ports()]
                    ),
                    value="disconnected",
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
                dbc.Label("Select Telecommand:", html_for="telecommand-dropdown"),
                dcc.Dropdown(
                    id="telecommand-dropdown",
                    options=[{"label": cmd, "value": cmd} for cmd in get_telecommand_name_list()],
                    value=get_telecommand_name_list()[0],
                    className="mb-3",  # Add margin bottom to the dropdown
                ),
            ],
        ),
        html.Div(id="argument-inputs", className="mb-3"),
        dbc.Button("Send", id="send-button", n_clicks=0, className="m-3"),
        dbc.Button(
            "Clear Arguments",
            id="clear-arguments-button",
            n_clicks=0,
            className="m-3",
        ),
    ]


def main() -> None:
    """Run the main Dash application."""
    app = dash.Dash(
        "CTS-SAT-1 Ground Support",
        external_stylesheets=[dbc.themes.BOOTSTRAP],
    )

    app.layout = dbc.Container(
        [
            html.H1("CTS-SAT-1 Ground Support - Telecommand Terminal", className="text-center"),
            dbc.Row(
                [
                    dbc.Col(
                        generate_left_pane(),
                        width=3,
                    ),
                    dbc.Col(
                        [
                            generate_rx_tx_log(),
                            dcc.Interval(
                                id="uart-update-interval-component",
                                interval=1000,  # in milliseconds
                                n_intervals=0,
                            ),
                        ],
                        width=9,
                    ),
                ],
            ),
        ],
        fluid=True,  # Use a fluid container for full width
    )

    start_uart_listener()

    app.run_server(debug=True)
    logger.info("Dash app started and finished.")


if __name__ == "__main__":
    main()
