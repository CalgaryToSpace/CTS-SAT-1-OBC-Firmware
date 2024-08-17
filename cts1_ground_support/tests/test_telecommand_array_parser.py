"""Unit tests for the `telecommand_array_parser.py` module."""

import pytest

from cts1_ground_support.telecommand_array_parser import (
    extract_c_function_docstring,
    extract_telecommand_arg_list,
    parse_telecommand_array_table,
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
    """Test the `parse_telecommand_array_table` function."""
    in1 = """
    const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 1,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    }};
    """

    expected1 = [
        TelecommandDefinition(
            name="hello_world",
            tcmd_func="TCMDEXEC_hello_world",
            number_of_args=0,
            readiness_level="TCMD_READINESS_LEVEL_FOR_OPERATION",
        ),
        TelecommandDefinition(
            name="heartbeat_off",
            tcmd_func="TCMDEXEC_heartbeat_off",
            number_of_args=1,
            readiness_level="TCMD_READINESS_LEVEL_FOR_OPERATION",
        ),
    ]
    parsed1 = parse_telecommand_array_table(in1)
    assert parsed1 == expected1

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_array_table("")

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_array_table(
            "const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {};",
        )

    with pytest.raises(ValueError, match="Expected to find exactly 1 telecommand array"):
        parse_telecommand_array_table("""
        const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions[] = {
        {
            .tcmd_name = "hello_world",
            .tcmd_func = TCMDEXEC_hello_world,
            .number_of_args = 0,
            .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
        },
        """)


def test_extract_c_function_docstring() -> None:
    """Test the `extract_c_function_docstring` function."""
    c_code = """
    /// @brief This is a docstring for the `hello_world` function.
    /// @param arg1 This is the first argument.
    /// @return This function returns 0.
    int hello_world(int arg1) {
        return 0;
    }

    /// @brief This is a docstring for another function.
    /// @param arg This is an argument for another function.
    /// @return This function returns 1.
    int another_function(int arg) {
        return 1;
    }
    """
    expected_docstring_hello_world = """
@brief This is a docstring for the `hello_world` function.
@param arg1 This is the first argument.
@return This function returns 0.""".strip()
    result_hello_world = extract_c_function_docstring("hello_world", c_code)
    assert result_hello_world == expected_docstring_hello_world

    expected_docstring_another_function = """
@brief This is a docstring for another function.
@param arg This is an argument for another function.
@return This function returns 1.""".strip()
    result_another_function = extract_c_function_docstring("another_function", c_code)
    assert result_another_function == expected_docstring_another_function

    no_docstring_result = extract_c_function_docstring("non_existent_function", c_code)
    assert no_docstring_result is None

    c_code_2 = """
void hello_world() {
    return 0;
}

/// @brief Telecommand: Erase a sector of flash memory.
/// @param args_str
/// - Arg 0: Chip Number (CS number) as uint
/// - Arg 1: Flash Address as uint
/// - Arg 2: Number of bytes to erase as uint
/// @return 0 on success, >0 on error
uint8_t TCMDEXEC_flash_erase(const char *args_str, TCMD_TelecommandChannel_enum_t tcmd_channel,
                        char *response_output_buf, uint16_t response_output_buf_len) {
    uint64_t chip_num, flash_addr;
}
    """
    expected_docstring_flash_erase = """
@brief Telecommand: Erase a sector of flash memory.
@param args_str
- Arg 0: Chip Number (CS number) as uint
- Arg 1: Flash Address as uint
- Arg 2: Number of bytes to erase as uint
@return 0 on success, >0 on error""".strip()
    result_flash_erase = extract_c_function_docstring("TCMDEXEC_flash_erase", c_code_2)
    assert result_flash_erase == expected_docstring_flash_erase


def test_extract_telecommand_arg_list() -> None:
    """Test the `extract_telecommand_arg_list` function."""
    input1 = """
    @brief Telecommand: Erase a sector of flash memory.
    @param args_str
    - Arg 0: Chip Number (CS number) as uint
    - Arg 1: Flash Address as uint
    - Arg 2: Number of bytes to erase as uint
    @return 0 on success, >0 on error
    """.strip()
    exp_output1 = [
        "Chip Number (CS number) as uint",
        "Flash Address as uint",
        "Number of bytes to erase as uint",
    ]
    result1 = extract_telecommand_arg_list(input1)
    assert result1 == exp_output1

    # No individual arguments, so return an empty list
    input2 = """
    @brief Telecommand: Erase a sector of flash memory.
    @param args_str No args. Ignored.
    @return 0 on success, >0 on error
    """
    assert extract_telecommand_arg_list(input2) == []

    # No `@param args_str` line, so return None
    input3 = """
@brief This is a docstring for the `hello_world` function.
@param arg1 This is the first argument.
@return This function returns 0.
    """
    assert extract_telecommand_arg_list(input3) is None

    # Similar case to 1, but without anything after the last argument description.
    input4 = """
    @brief Telecommand: Erase a sector of flash memory.
    @param args_str
    - Arg 0: Chip Number (CS number) as uint
    - Arg 1: Flash Address as uint
    - Arg 2: Number of bytes to erase as uint
    """.strip()
    exp_output4 = [
        "Chip Number (CS number) as uint",
        "Flash Address as uint",
        "Number of bytes to erase as uint",
    ]
    result4 = extract_telecommand_arg_list(input4)
    assert result4 == exp_output4


def test_parse_telecommand_list_from_repo() -> None:
    """Test the `parse_telecommand_list_from_repo` function with the real file in this repo."""
    telecommands = parse_telecommand_list_from_repo()

    assert isinstance(telecommands, list)
    assert len(telecommands) > 5

    # Check that we have the hello_world telecommand.
    found_hello_world_tcmds = [tcmd for tcmd in telecommands if tcmd.name == "hello_world"]
    assert (
        len(found_hello_world_tcmds) == 1
    ), f"Expected to find 1 hello_world telecommand, but found {len(found_hello_world_tcmds)}"
    hello_world_telecommand = found_hello_world_tcmds[0]
    assert hello_world_telecommand.name == "hello_world"
    assert hello_world_telecommand.tcmd_func == "TCMDEXEC_hello_world"
    assert hello_world_telecommand.number_of_args == 0
    assert hello_world_telecommand.full_docstring.startswith("@brief ")
    assert hello_world_telecommand.argument_descriptions == []

    # Check a telecommand with 1 argument.
    found_read_file_tcmds = [tcmd for tcmd in telecommands if tcmd.name == "fs_read_file_hex"]
    assert (
        len(found_read_file_tcmds) == 1
    ), f"Expected to find 1 fs_read_file_hex telecommand, but found {len(found_read_file_tcmds)}"
    read_file_telecommand = found_read_file_tcmds[0]
    assert read_file_telecommand.name == "fs_read_file_hex"
    assert read_file_telecommand.tcmd_func == "TCMDEXEC_fs_read_file_hex"
    assert read_file_telecommand.number_of_args == 1
    assert read_file_telecommand.full_docstring.startswith("@brief ")
    assert read_file_telecommand.argument_descriptions == [
        "File path as string",
    ]

    # Check a telecommand with 2 arguments.
    found_read_file_tcmds = [tcmd for tcmd in telecommands if tcmd.name == "fs_write_file"]
    assert (
        len(found_read_file_tcmds) == 1
    ), f"Expected to find 1 fs_read_file_hex telecommand, but found {len(found_read_file_tcmds)}"
    read_file_telecommand = found_read_file_tcmds[0]
    assert read_file_telecommand.name == "fs_write_file"
    assert read_file_telecommand.tcmd_func == "TCMDEXEC_fs_write_file"
    assert read_file_telecommand.number_of_args == 2
    assert read_file_telecommand.full_docstring.startswith("@brief ")
    assert read_file_telecommand.argument_descriptions == [
        "File path as string",
        "String to write to file",
    ]
