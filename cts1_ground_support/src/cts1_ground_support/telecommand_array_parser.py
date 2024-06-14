"""A set of tools to read the list of telecommands from the `telecommand_definitions.c` file."""

import json
import re
from pathlib import Path

from cts1_ground_support.paths import get_repo_path
from cts1_ground_support.telecommand_types import TelecommandDefinition


def remove_c_comments(text: str) -> str:
    """Remove C-style comments from a string.

    Note: This function is not perfect, and fails with certain cases.
    """
    text = re.sub(r"\s*/\*.*?\*/\s*", "\n", text, flags=re.DOTALL)  # DOTALL makes . match newlines
    return re.sub(r"\s*//.*", "", text)


def parse_telecommand_list(c_code: str | Path) -> list[TelecommandDefinition]:
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
    },
    {
        .tcmd_name = "heartbeat_off",
        .tcmd_func = TCMDEXEC_heartbeat_off,
        .number_of_args = 0,
    } } ;
    ```

    """
    if isinstance(c_code, Path):
        c_code = c_code.read_text()

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
            ),
        )

    return telecommands


def parse_telecommand_list_from_repo() -> list[TelecommandDefinition]:
    """Read the list of telecommands from the `telecommand_definitions.c` file in the repo.

    This type of function definitely has a code smell, but it's the main function here, so it's
    fine.
    """
    telecommands_defs_path = (
        get_repo_path() / "firmware/Core/Src/telecommands/telecommand_definitions.c"
    )
    return parse_telecommand_list(telecommands_defs_path)


if __name__ == "__main__":
    # Do a demo of the `parse_telecommand_list` function
    telecommands = parse_telecommand_list_from_repo()
    print(json.dumps([tcmd.to_dict() for tcmd in telecommands], indent=2))  # noqa: T201
