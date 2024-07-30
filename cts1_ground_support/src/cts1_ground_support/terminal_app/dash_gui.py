"""The main screen GUI, which allows sending commands, and viewing the RX/TX log.

The main screen has the following components:
- Left Side: The list of commands, as a pick list.
- Left Side: Text fields to input each argument for the command.
- Right Side: An "RX/TX" log, which shows the most recent received and transmitted messages
(occupies the right 70% of the screen).
"""

import argparse
import functools
import json
import time

import dash
import dash_bootstrap_components as dbc
import dash_split_pane
from dash import callback, dcc, html
from dash.dependencies import Input, Output, State
from loguru import logger

from cts1_ground_support.serial import list_serial_ports
from cts1_ground_support.telecommand_array_parser import parse_telecommand_list_from_repo
from cts1_ground_support.telecommand_preview import generate_telecommand_preview
from cts1_ground_support.telecommand_types import TelecommandDefinition
from cts1_ground_support.terminal_app.app_store import app_store
from cts1_ground_support.terminal_app.app_types import UART_PORT_NAME_DISCONNECTED, RxTxLogEntry
from cts1_ground_support.terminal_app.serial_thread import start_uart_listener

UART_PORT_OPTION_LABEL_DISCONNECTED = "⛔ Disconnected ⛔"

# TODO: log the UART comms to a file
# TODO: fix the connect/disconnect loop when multiple clients are connected
#   ^ Ideas: Add an "Apply" button to change the port, or a mechanism where the "default-on-load"
#            value comes from the app_store's latest connected port (esp on load).


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
    Output("argument-inputs-container", "children"),
    Input("telecommand-dropdown", "value"),
)
def update_argument_inputs(selected_command_name: str) -> list[html.Div]:
    """Generate the argument input fields based on the selected telecommand."""
    selected_tcmd = get_telecommand_by_name(selected_command_name)

    arg_inputs = []
    for arg_num in range(get_max_arguments_per_telecommand()):
        if (selected_tcmd.argument_descriptions is not None) and (
            arg_num < len(selected_tcmd.argument_descriptions)
        ):
            arg_description = selected_tcmd.argument_descriptions[arg_num]
            label = f"Arg {arg_num}: {arg_description}"
        else:
            label = f"Arg {arg_num}"

        this_id = f"arg-input-{arg_num}"

        arg_inputs.append(
            dbc.FormFloating(
                [
                    dbc.Input(
                        type="text",
                        id=this_id,
                        placeholder=label,
                        disabled=(arg_num >= selected_tcmd.number_of_args),
                        style={"fontFamily": "monospace"},
                    ),
                    dbc.Label(label, html_for=this_id),
                ],
                className="mb-3",
                # Hide the argument input if it is not needed for the selected telecommand.
                style=({"display": "none"} if arg_num >= selected_tcmd.number_of_args else {}),
            )
        )

    return arg_inputs


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
    Output("stored-command-preview", "data"),
    Input("telecommand-dropdown", "value"),
    Input("suffix-tags-checklist", "value"),
    Input("input-tsexec-suffix-tag", "value"),
    Input("extra-suffix-tags-input", "value"),  # Advanced feature for debugging
    Input("uart-update-interval-component", "n_intervals"),
    # TODO: maybe this could be cleaner with `Input/State("argument-inputs-container", "children")`
    *[
        Input(f"arg-input-{arg_num}", "value")
        for arg_num in range(get_max_arguments_per_telecommand())
    ],
    prevent_initial_call=True,  # Objects aren't created yet, so errors are thrown.
)
def update_stored_command_preview(
    selected_command_name: str,
    suffix_tags_checklist: list[str] | None,
    tsexec_suffix_tag: str | None,
    extra_suffix_tags_input: str,
    _n_intervals: int,
    *every_arg_value: tuple[str],
) -> list:
    """When any input to the command preview changes, regenerate the command preview.

    Stores the command preview so that it's accessible from any function which wants it.
    """
    # Prep incoming args.
    if suffix_tags_checklist is None:
        suffix_tags_checklist = []

    if tsexec_suffix_tag == "":
        tsexec_suffix_tag = None

    # Get the selected command and its arguments.
    selected_command = get_telecommand_by_name(selected_command_name)
    arg_vals = [every_arg_value[arg_num] for arg_num in range(selected_command.number_of_args)]

    # Replace None with empty string, to avoid "None" in the preview.
    arg_vals: list[str] = [str(arg) if arg is not None else "" for arg in arg_vals]

    enable_tssent_suffix = "enable_tssent_tag" in suffix_tags_checklist

    extra_suffix_tags = {}
    if extra_suffix_tags_input:
        try:
            extra_suffix_tags = json.loads(extra_suffix_tags_input)
        except json.JSONDecodeError as e:
            logger.error(f"Error decoding JSON in extra-suffix-tags-input field: {e}")

        if not isinstance(extra_suffix_tags, dict):
            logger.error(f"Extra suffix tags input is not a dictionary: {extra_suffix_tags}")
            extra_suffix_tags = {}

    return generate_telecommand_preview(
        tcmd_name=selected_command_name,
        arg_list=arg_vals,
        enable_tssent_suffix=enable_tssent_suffix,
        tsexec_suffix_value=tsexec_suffix_tag,
        extra_suffix_tags=extra_suffix_tags.copy(),
    )


@callback(
    Output("command-preview-container", "children"),
    Input("stored-command-preview", "data"),
)
def update_command_preview_render(command_preview: str) -> list:
    """Make an area with the command preview for the selected telecommand."""
    return [
        html.H4(["Command Preview"], className="text-center"),
        html.Pre(command_preview, id="command-preview", className="mb-3"),
    ]


@callback(
    Input("send-button", "n_clicks"),
    State("telecommand-dropdown", "value"),
    State("stored-command-preview", "data"),
    # TODO: maybe this could be cleaner with `Input/State("argument-inputs-container", "children")`
    *[
        State(f"arg-input-{arg_num}", "value")
        for arg_num in range(get_max_arguments_per_telecommand())
    ],
    prevent_initial_call=True,
)
def send_button_callback(
    n_clicks: int, selected_command_name: str, command_preview: str, *every_arg_value: tuple[str]
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

    logger.info(f"Adding command to queue: {command_preview}")

    app_store.last_tx_timestamp_sec = time.time()
    app_store.tx_queue.append(command_preview.encode("ascii"))


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
def update_uart_port_dropdown_options(uart_port_name: str | None, _n_intervals: int) -> list[str]:
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


@callback(
    Output("selected-tcmd-info-container", "children"),
    Input("telecommand-dropdown", "value"),
)
def update_selected_tcmd_info(selected_command_name: str) -> list:
    """Make an area with the docstring for the selected telecommand."""
    selected_command = get_telecommand_by_name(selected_command_name)

    if selected_command.full_docstring is None:
        docstring = f"No docstring found for {selected_command.tcmd_func}"
    else:
        docstring = selected_command.full_docstring

    table_fields = selected_command.to_dict_table_fields()

    table_header = html.Thead(html.Tr([html.Th("Field"), html.Th("Value")]))
    table_body = html.Tbody(
        [
            html.Tr(
                [
                    html.Td(key),
                    html.Td(value, style={"fontFamily": "monospace"}),
                ]
            )
            for key, value in table_fields.items()
        ]
    )

    table = dbc.Table(
        [table_header, table_body], bordered=True, striped=True, hover=True, responsive=True
    )

    return [
        html.H4(["Command Info"], className="text-center"),
        table,
        html.Hr(),
        html.H4(["Command Docstring"], className="text-center"),
        # TODO: add the "brief" docstring here, and then hide the rest in a "Click to expand"
        html.Pre(docstring, id="selected-tcmd-info", className="mb-3"),
    ]


def generate_rx_tx_log(
    *, show_end_of_line_chars: bool = False, show_timestamp: bool = False
) -> html.Div:
    """Generate the RX/TX log, which shows the most recent received and transmitted messages."""
    return html.Div(
        [
            html.Pre(
                entry.to_string(
                    show_end_of_line_chars=show_end_of_line_chars, show_timestamp=show_timestamp
                ),
                style=(
                    entry.css_style
                    | {"margin": "0", "lineHeight": "1.1", "whiteSpace": "pre-wrap"}
                ),
            )
            for entry in app_store.rxtx_log
        ],
        id="rx-tx-log",
        className="p-3",
    )


# Should be the last callback in the file, as other callbacks modify the log.
@callback(
    Output("rx-tx-log-container", "children"),
    Output("uart-update-interval-component", "interval"),
    Input("uart-port-dropdown", "value"),
    Input("send-button", "n_clicks"),
    Input("clear-log-button", "n_clicks"),
    Input("uart-update-interval-component", "n_intervals"),
    Input("display-options-checklist", "value"),
)
def update_uart_log_interval(
    _uart_port_name: str,
    _n_clicks_send: int,
    _n_clicks_clear_logs: int,
    _update_interval_count: int,
    display_options_checklist: list[str] | None,
) -> html.Div:
    """Update the UART log at the specified interval. Also, update the refresh interval."""
    sec_since_send = time.time() - app_store.last_tx_timestamp_sec
    if sec_since_send < 10:  # noqa: PLR2004
        # Rapid refreshed right after sending a command.
        app_store.uart_log_refresh_rate_ms = 250
    elif sec_since_send < 60:  # noqa: PLR2004
        # Chill if it's been a while since the last command.
        app_store.uart_log_refresh_rate_ms = 800
    else:
        # Slow down if it's been a long time since the last command.
        app_store.uart_log_refresh_rate_ms = 2000

    if display_options_checklist:
        show_end_of_line_chars = "show_end_of_line_chars" in display_options_checklist
        show_timestamp = "show_timestamp" in display_options_checklist
    else:
        show_end_of_line_chars = False
        show_timestamp = False

    return (
        # new log entries
        generate_rx_tx_log(
            show_end_of_line_chars=show_end_of_line_chars, show_timestamp=show_timestamp
        ),
        app_store.uart_log_refresh_rate_ms,  # new refresh interval
    )


def generate_left_pane(*, selected_command_name: str, enable_advanced: bool) -> list:
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
        html.Hr(),
        dbc.Row(
            [
                dbc.Label("Select a Telecommand:", html_for="telecommand-dropdown"),
                dcc.Dropdown(
                    id="telecommand-dropdown",
                    options=[{"label": cmd, "value": cmd} for cmd in get_telecommand_name_list()],
                    value=selected_command_name,
                    className="mb-3",  # Add margin bottom to the dropdown
                    style={"fontFamily": "monospace"},
                ),
            ],
        ),
        html.Div(
            update_argument_inputs(selected_command_name),
            id="argument-inputs-container",
            className="mb-3",
        ),
        html.Hr(),
        html.Div(
            [
                dbc.Label("Suffix Tag Options:", html_for="suffix-tags-checklist"),
                dbc.Checklist(
                    options={
                        "enable_tssent_tag": "Send '@tssent=current_timestamp' Tag?",
                        # TODO: add more here, like the "Send 'sha256' Tag"
                    },
                    id="suffix-tags-checklist",
                ),
            ]
        ),
        html.Div(
            dbc.FormFloating(
                [
                    dbc.Input(
                        type="text",
                        id="input-tsexec-suffix-tag",
                        placeholder="Timestamp to Execute Command (@tsexec=xxx)",
                        style={"fontFamily": "monospace"},
                    ),
                    dbc.Label(
                        "Timestamp to Execute Command (@tsexec=xxx)",
                        html_for="input-tsexec-suffix-tag",
                    ),
                ],
                className="mb-3",
            ),
        ),
        html.Div(
            dbc.FormFloating(
                [
                    dbc.Input(
                        type="text",
                        id="extra-suffix-tags-input",
                        placeholder="Extra Suffix Tags Input (JSON)",
                        style={"fontFamily": "monospace"},
                    ),
                    dbc.Label(
                        "Extra Suffix Tags Input (JSON)", html_for="extra-suffix-tags-input"
                    ),
                ],
                className="mb-3",
                # Hide this field by default, and only show if the CLI arg "--advanced" is passed.
                style=({} if enable_advanced else {"display": "none"}),
            ),
        ),
        html.Hr(),
        html.Div(id="command-preview-container", className="mb-3"),
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
        html.Hr(),
        html.Div(id="selected-tcmd-info-container", className="mb-3"),
        html.Hr(),
        html.H4("Display Options", className="text-center"),
        html.Div(
            [
                dbc.Checklist(
                    options={
                        "show_end_of_line_chars": "Show End-of-Line Characters?",
                        "show_timestamp": "Show Timestamps?",
                    },
                    id="display-options-checklist",
                ),
            ]
        ),
    ]


def run_dash_app(*, enable_debug: bool = False, enable_advanced: bool = False) -> None:
    """Run the main Dash application."""
    app_name = "CTS-SAT-1 Ground Support"
    app = dash.Dash(
        __name__,  # required to load /assets folder
        external_stylesheets=[dbc.themes.BOOTSTRAP],
        title=app_name,
        update_title=(
            # Disable the update title, unless we're debugging.
            # Makes it look cleaner overall.
            "Updating..." if enable_debug else None
        ),
    )

    app.layout = dbc.Container(
        [
            dash_split_pane.DashSplitPane(
                [
                    html.Div(
                        generate_left_pane(
                            selected_command_name=get_telecommand_name_list()[0],  # default
                            enable_advanced=enable_advanced,
                        ),
                        className="p-3",
                        style={
                            "height": "100%",
                            "overflowY": "auto",  # Enable vertical scroll.
                        },
                    ),
                    html.Div(
                        generate_rx_tx_log(),
                        id="rx-tx-log-container",
                        style={
                            "fontFamily": "monospace",
                            "backgroundColor": "black",
                            "height": "100%",
                            "overflowY": "auto",
                            "flexDirection": "column-reverse",
                            "display": "flex",
                        },
                    ),
                ],
                id="vertical-split-pane-1",
                split="vertical",
                size=500,  # Default starting size.
                minSize=300,
            ),
            dbc.Button(
                "Jump to Bottom ⬇️",
                id="scroll-to-bottom-button",
                style={
                    "display": "none",
                    "position": "fixed",
                    "bottom": "20px",
                    "right": "60px",
                    "zIndex": "99",
                },
                color="danger",
            ),
            dcc.Interval(
                id="uart-update-interval-component",
                interval=800,  # in milliseconds
                n_intervals=0,
            ),
            dcc.Store(id="stored-command-preview", data=""),
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
    parser.add_argument(
        "-a",
        "--advanced",
        action="store_true",
        help="Enable advanced features for ground debugging, like the extra suffix tags input.",
    )
    args = parser.parse_args()
    run_dash_app(enable_debug=args.debug, enable_advanced=args.advanced)


if __name__ == "__main__":
    main()
