"""Unit tests for the `telecommand_array_parser.py` module."""

import pytest
from cts1_ground_support.telecommand_array_parser import (
    parse_telecommand_list,
    parse_telecommand_list_from_repo,
    remove_c_comments,
)
from cts1_ground_support.telecommand_types import TelecommandDefinition


def test_remove_c_comments() -> None:
    """Test the `remove_c_comments` function."""
    in1 = """int var = 5; // This is a comment"""
    assert remove_c_comments(in1) == "int var = 5;"

    in2 = """int var = 5; /* This is a comment */ int var2 = 6;"""
    assert remove_c_comments(in2) == "int var = 5;\nint var2 = 6;"


def test_parse_telecommand_list() -> None:
    """Test the `parse_telecommand_list` function."""
    in1 = """
    const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
    },
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 1,
    }};
    """

    expected1 = [
        TelecommandDefinition(
            name="hello_world",
            tcmd_func="TCMDEXEC_hello_world",
            number_of_args=0,
        ),
        TelecommandDefinition(
            name="heartbeat_off",
            tcmd_func="TCMDEXEC_heartbeat_off",
            number_of_args=1,
        ),
    ]
    parsed1 = parse_telecommand_list(in1)
    assert parsed1 == expected1

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_list("")

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_list(
            "const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {};",
        )

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_list("""
        const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
        {
            .tcmd_name = "hello_world",
            .tcmd_func = TCMDEXEC_hello_world,
            .number_of_args = 0,
        },
        """)


def test_parse_telecommand_list_from_repo() -> None:
    """Test the `parse_telecommand_list_from_repo` function with the real file in this repo."""
    telecommands = parse_telecommand_list_from_repo()

    assert isinstance(telecommands, list)
    assert len(telecommands) > 5

    hello_world_telecommand = TelecommandDefinition(
        name="hello_world",
        tcmd_func="TCMDEXEC_hello_world",
        number_of_args=0,
    )
    assert hello_world_telecommand in telecommands
