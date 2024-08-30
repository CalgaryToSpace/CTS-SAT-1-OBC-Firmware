"""Check that all `TEST_EXEC_` unit test function definitions are registered in the table."""

import re
import sys
from pathlib import Path

import git
from loguru import logger

GIT_REPO_ROOT_PATH = Path(git.Repo(__file__, search_parent_directories=True).working_tree_dir)
TEST_TABLE_FILE_PATH = "firmware/Core/Src/unit_tests/unit_test_inventory.c"


def get_test_exec_function_name_definition_list() -> list[str]:
    """Get the list of all `TEST_EXEC_` function names in the repository."""
    pattern = re.compile(r"int.{0,30}(?P<name>TEST_EXEC_\w+)", flags=re.DOTALL)

    function_name_list: list[str] = []

    for file_path in GIT_REPO_ROOT_PATH.rglob("*.c"):
        file_contents = file_path.read_text()

        # Find all `TEST_EXEC_` function definitions
        for match in pattern.finditer(file_contents):
            function_name = match.group("name")

            function_name_list.append(function_name)

    return function_name_list


def get_test_exec_registered_function_name_list() -> list[str]:
    """Get the list of all `TEST_EXEC_` function names in the table."""
    # Matches like: ".test_func = TEST_EXEC__test_asserts,"
    pattern = re.compile(r"\.test_func = (?P<name>TEST_EXEC_\w+),")

    function_name_list = []

    file_path = GIT_REPO_ROOT_PATH / TEST_TABLE_FILE_PATH

    with file_path.open("r") as f:
        for line in f:
            match = pattern.search(line)

            if match:
                function_name = match.group("name")

                function_name_list.append(function_name)

    return function_name_list


def validate_test_exec_registration() -> None:
    """Check that all `TEST_EXEC_` functions are registered in the table."""
    test_exec_function_name_list = get_test_exec_function_name_definition_list()
    logger.info(f"Found {len(test_exec_function_name_list)} `TEST_EXEC_` function definitions.")

    test_exec_registered_function_name_list = get_test_exec_registered_function_name_list()
    logger.info(f"Found {len(test_exec_registered_function_name_list)} `TEST_EXEC_` functions.")

    unregistered_functions = set(test_exec_function_name_list) - set(
        test_exec_registered_function_name_list
    )
    registered_but_dont_exist = set(test_exec_registered_function_name_list) - set(
        test_exec_function_name_list
    )

    issue_count = 0
    if registered_but_dont_exist:
        logger.error(
            "There are `TEST_EXEC_` functions registered in the table for which the function "
            "definition does not exist. If the code builds, this likely indicates a bug here. "
            "Functions which are registered but do not exist in the table: "
            f"{list(registered_but_dont_exist)}"
        )

        issue_count += 1

    if unregistered_functions:
        logger.error(
            "Not all `TEST_EXEC_` functions are registered in the table. "
            f"Please add them to: {TEST_TABLE_FILE_PATH}. "
            f"Unregistered functions: {list(unregistered_functions)}"
        )

        issue_count += 1

    if issue_count:
        sys.exit(issue_count)


if __name__ == "__main__":
    validate_test_exec_registration()
