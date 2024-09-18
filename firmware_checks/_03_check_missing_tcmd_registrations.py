"""Check that all `TCMDEXEC_` function definitions are registered in the table."""

import re
import sys
from pathlib import Path

import git
from loguru import logger

from cts1_ground_support.telecommand_array_parser import parse_telecommand_list_from_repo

GIT_REPO_ROOT_PATH = Path(git.Repo(__file__, search_parent_directories=True).working_tree_dir)


def get_tcmdexec_function_name_definition_list() -> list[str]:
    """Get the list of all `TCMDEXEC_` function names in the repository."""
    pattern = re.compile(r"int.{0,10}(?P<name>TCMDEXEC_\w+)", flags=re.DOTALL)

    function_name_list: list[str] = []

    for file_path in GIT_REPO_ROOT_PATH.rglob("*.c"):
        file_contents = file_path.read_text()

        for match in pattern.finditer(file_contents):
            function_name = match.group("name")
            function_name_list.append(function_name)

    return function_name_list


def get_tcmdexec_registered_function_name_list() -> list[str]:
    """Get the list of all `TCMDEXEC_` function names in the table."""
    tcmd_list = parse_telecommand_list_from_repo()

    return [tcmd.tcmd_func for tcmd in tcmd_list]


def validate_tcmdexec_registration() -> None:
    """Check that all `TCMDEXEC_` functions are registered in the table."""
    issue_count = 0

    defined_function_name_list = get_tcmdexec_function_name_definition_list()
    logger.info(f"Found {len(defined_function_name_list)} `TCMDEXEC_` function definitions.")

    if len(set(defined_function_name_list)) != len(defined_function_name_list):
        duplicate_definition_function_names = {
            x for x in defined_function_name_list if defined_function_name_list.count(x) > 1
        }
        logger.warning(
            "There are duplicate `TCMDEXEC_` function definitions. This should be a build error. "
            f"Functions with multiple definitions: {list(duplicate_definition_function_names)}"
        )
        issue_count += 1

    registered_function_name_list = get_tcmdexec_registered_function_name_list()
    logger.info(f"Found {len(registered_function_name_list)} `TCMDEXEC_` functions registered.")

    if len(set(registered_function_name_list)) != len(registered_function_name_list):
        duplicate_registration_function_names = {
            x for x in registered_function_name_list if registered_function_name_list.count(x) > 1
        }
        logger.warning(
            "There are duplicate `TCMDEXEC_` function registrations in the "
            "`telecommand_definitions.c` table. "
            f"Duplicated functions: {list(duplicate_registration_function_names)}"
        )
        issue_count += 1

    unregistered_functions = set(defined_function_name_list) - set(registered_function_name_list)
    registered_but_dont_exist = set(registered_function_name_list) - set(
        defined_function_name_list
    )

    if registered_but_dont_exist:
        logger.error(
            "There are `TCMDEXEC_` functions registered in the table for which the function "
            "definition does not exist. If the code builds, this likely indicates a bug here. "
            "Functions which are registered but do not exist in the table: "
            f"{list(registered_but_dont_exist)}"
        )

        issue_count += 1

    if unregistered_functions:
        logger.error(
            "Not all `TCMDEXEC_` functions are registered in the table. "
            "Please add them to: `telecommand_definitions.c`. "
            f"Unregistered functions: {list(unregistered_functions)}"
        )

        issue_count += 1

    if issue_count:
        sys.exit(issue_count)
    else:
        logger.success("All `TCMDEXEC_` functions are registered properly.")


if __name__ == "__main__":
    validate_tcmdexec_registration()
