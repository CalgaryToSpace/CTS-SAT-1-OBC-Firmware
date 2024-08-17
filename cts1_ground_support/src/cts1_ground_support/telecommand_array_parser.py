"""A set of tools to read the list of telecommands from the `telecommand_definitions.c` file."""

import json
import re
from pathlib import Path

from cts1_ground_support.paths import get_repo_path, read_text_file
from cts1_ground_support.telecommand_types import TelecommandDefinition


def remove_c_comments(text: str) -> str:
    """Remove C-style comments from a string.

    Note: This function is not perfect, and fails with certain cases.
    """
    text = re.sub(r"\s*/\*.*?\*/\s*", "\n", text, flags=re.DOTALL)  # DOTALL makes . match newlines
    return re.sub(r"\s*//.*", "", text)


def parse_telecommand_array_table(c_code: str | Path) -> list[TelecommandDefinition]:
    """Read the list of telecommands from a file like the `telecommand_definitions.c` file.

    Args:
    ----
        c_code: The C code containing the telecommand definitions, or a path to a file containing
        the code.

    Example Input:
    -------------

    ```c
    const TCMD_TelecommandDefinition_t TCMD_telecommand_definitions [ ] = {
    {
        .tcmd_name = "hello_world",
        .tcmd_func = TCMDEXEC_hello_world,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    },
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 0,
        .readiness_level = TCMD_READINESS_LEVEL_FOR_OPERATION,
    } } ;
    ```

    """
    if isinstance(c_code, Path):
        c_code = read_text_file(c_code)

    c_code = remove_c_comments(c_code)

    top_level_regex = re.compile(
        r"TCMD_TelecommandDefinition_t\s+\w+\s*\[\s*\]\s*=\s*{"
        r"(?P<all_struct_declarations>(\s*{\s*[^{}]+\s*},?)+)"
        r"\s*};",
    )

    # Use the `struct_body_regex` to extract the body of a struct declaration.
    struct_body_regex = re.compile(r"{\s*(?P<struct_body>[^{}]+)\s*}")

    # Use the `struct_level_regex` on a "struct_declaration" from the `top_level_regex` to extract
    # the individual struct fields.
    struct_level_regex = re.compile(r"\s*\.(?P<field_name>\w+)\s*=\s*(?P<field_value>[^,]+),?")

    telecommands: list[TelecommandDefinition] = []

    top_level_matches = list(top_level_regex.finditer(c_code))
    if len(top_level_matches) != 1:
        msg = (
            f"Expected to find exactly 1 telecommand array in the input code, but found "
            f"{len(top_level_matches)} matches."
        )
        raise ValueError(msg)

    top_level_match = top_level_matches[0]
    all_struct_declarations = top_level_match.group("all_struct_declarations")

    for struct_declaration in re.finditer(struct_body_regex, all_struct_declarations):
        struct_body = struct_declaration.group("struct_body")

        fields: dict[str, str | int] = {}
        for struct_match in struct_level_regex.finditer(struct_body):
            field_name = struct_match.group("field_name")
            field_value = struct_match.group("field_value").strip().strip('"')

            # Convert int field(s) to int
            if field_name in ["number_of_args"]:
                field_value = int(field_value)

            fields[field_name] = field_value

        telecommands.append(
            TelecommandDefinition(
                name=fields["tcmd_name"],
                tcmd_func=fields["tcmd_func"],
                number_of_args=fields["number_of_args"],
                readiness_level=fields["readiness_level"],
            ),
        )

    return telecommands


def extract_c_function_docstring(function_name: str, c_code: str) -> str | None:
    """Read the docstring for the specified function, and extract the description of each argument.

    Does not support `/* */` style comments, only `///` style comments.

    Args:
    ----
        function_name: The name of the function to extract the docstring for.
        c_code: The C code containing the function definitions. Can be multiple files worth.

    Returns:
    -------
        The docstring for the function, with the `///` characters removed. If the function is not
        found, returns None.

    Example:
    -------
    In the following example, the output is the comment section before the `hello_world` function,
    without the leading `///` characters.

    ```c
    /// @brief This is a docstring for the `hello_world` function.
    /// @param arg1 This is the first argument.
    /// @return This function returns 0.
    int hello_world(int arg1) {
        return 0;
    }
    ```

    """
    pattern = re.compile(
        rf"(?P<docstring>(///(.*)\s*)+)\s*(?P<return_type>\w+)\s+{function_name}\s*\("
    )
    match = pattern.search(c_code)
    if match:
        docstring = match.group("docstring")
        docstring_lines = [
            line.strip().lstrip("/").strip() for line in docstring.strip().split("\n")
        ]
        return "\n".join(docstring_lines)

    # If the function is not found, return None
    return None


def extract_telecommand_arg_list(docstring: str) -> list[str]:
    """Extract the list of argument descriptions from a telecommand docstring.

    Args:
    ----
        docstring: The docstring for a telecommand function, as extracted by
        `extract_c_function_docstring`. Can be prefixed with comment characters or not.

    Returns:
    -------
        A list of argument descriptions, starting with Arg 0 and increasing.
        Returns an empty list if there are no broken-out arguments
        Returns None if the input docstring does not contain a `@param args_str` section.

    Example:
    -------
    Input:

    ```text
    @brief Telecommand: Erase a sector of flash memory.
    @param args_str
    - Arg 0: Chip Number (CS number) as uint
    - Arg 1: Flash Address as uint
    - Arg 2: Number of bytes to erase as uint
    @return 0 on success, >0 on error
    ```

    Output:
    ```json
    [
        "Chip Number (CS number) as uint",
        "Flash Address as uint",
        "Number of bytes to erase as uint"
    ]
    ```

    """
    arg_pattern = re.compile(
        r"@param args_str.*\n(?P<args>([\s/]*- Arg (?P<arg_num>\d+): (?P<arg_description>.+)\s*)*)"
    )

    matches = []
    match = arg_pattern.search(docstring)
    if not match:
        return None

    args_text = match.group("args")
    arg_desc_pattern = re.compile(r"- Arg (?P<arg_num>\d+): (?P<arg_description>.+)\s*")

    matches.extend(
        [arg_match.group("arg_description") for arg_match in arg_desc_pattern.finditer(args_text)]
    )

    return matches


def parse_telecommand_list_from_repo(repo_path: Path | None = None) -> list[TelecommandDefinition]:
    """Read the list of telecommands array table and extract docstrings for each telecommand.

    Args:
    ----
        repo_path: The path to the root of the repository. If None, the path is set automatically.

    """
    if repo_path is None:
        repo_path = get_repo_path()

    # Assert that the input is a Path object.
    if not isinstance(repo_path, Path):
        msg = f"Expected a Path object, but got {type(repo_path)}"
        raise TypeError(msg)
    # Assert that the input is a directory.
    if not repo_path.is_dir():
        msg = f"Expected a directory, but got {repo_path}"
        raise ValueError(msg)

    telecommands_defs_path = repo_path / "firmware/Core/Src/telecommands/telecommand_definitions.c"

    # Assert that the file exists.
    if not telecommands_defs_path.is_file():
        msg = "The telecommand definitions file does not exist in the expected location."
        raise ValueError(msg)

    tcmd_list = parse_telecommand_array_table(telecommands_defs_path)

    # Read the files which may contain the TCMDEXEC functions.
    c_files_concat: str = "\n".join(
        read_text_file(f) for f in repo_path.glob("firmware/Core/Src/**/*.c")
    )

    # Extract the docstrings for each telecommand.
    for tcmd_idx in range(len(tcmd_list)):
        docstring = extract_c_function_docstring(
            tcmd_list[tcmd_idx].tcmd_func,
            c_files_concat,
        )
        if docstring is not None:
            tcmd_list[tcmd_idx].full_docstring = docstring
            tcmd_list[tcmd_idx].argument_descriptions = extract_telecommand_arg_list(docstring)

    return tcmd_list


if __name__ == "__main__":
    # Do a demo of the `parse_telecommand_array_table` function
    telecommands = parse_telecommand_list_from_repo()
    print(json.dumps([tcmd.to_dict() for tcmd in telecommands], indent=2))  # noqa: T201
