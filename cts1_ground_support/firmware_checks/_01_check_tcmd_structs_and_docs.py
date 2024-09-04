"""Check that the telecommand definitions are good."""

import sys

from loguru import logger

from cts1_ground_support.telecommand_array_parser import parse_telecommand_list_from_repo


def check_tcmd_struct_fields() -> None:
    """Check that the telecommand struct fields are all present."""
    success = True
    tcmd_list = parse_telecommand_list_from_repo()

    for tcmd in tcmd_list:
        if not tcmd.has_required_fields():
            logger.warning(
                f"The struct for the '{tcmd.name}' telecommand is missing required fields."
            )
            success = False

    if success:
        logger.success(f"All {len(tcmd_list):,} telecommand structs have the required fields.")
    else:
        msg = "One or more telecommand structs are missing required fields."
        logger.error(msg)
        sys.exit(1)


def check_tcmd_arg_lists() -> None:
    """Check that the telecommand docstrings have the correct number of arguments."""
    success = True
    tcmd_list = parse_telecommand_list_from_repo()

    for tcmd in tcmd_list:
        if tcmd.argument_descriptions is None:
            if tcmd.number_of_args > 0:
                logger.warning(
                    f"The '{tcmd.name}' telecommand is missing the `args_str` param in its "
                    "docstring."
                )
                success = False

            # Can't continue on to the next check if the argument descriptions are missing.
            continue

        if tcmd.number_of_args != len(tcmd.argument_descriptions):
            logger.warning(
                f"The '{tcmd.name}' telecommand has number_of_args={tcmd.number_of_args} args in "
                f"the struct, and {len(tcmd.argument_descriptions)} args in the docstring. "
                "These should be equal."
            )
            success = False

    if success:
        logger.success(f"All {len(tcmd_list)} telecommands have appropriate argument docstrings!")
    else:
        logger.error(
            "Please look at the `docs/C_General_Guidelines.md` file to see the docstring "
            "specification for telecommands."
        )
        msg = "One or more telecommand structs are missing required fields."
        logger.error(msg)
        sys.exit(1)


def check_tcmd_function_names_match_registration_names() -> None:
    """Check that the telecommand function names match the registration names."""
    error_count = 0
    tcmd_list = parse_telecommand_list_from_repo()

    for tcmd in tcmd_list:
        if tcmd.tcmd_func != f"TCMDEXEC_{tcmd.name}":
            logger.warning(
                f"The '{tcmd.name}' telecommand has a function name that does not match its "
                f"registration name. The function name is `{tcmd.tcmd_func}`, but the "
                f"registration name is `{tcmd.name}`."
            )
            error_count += 1

    if error_count == 0:
        logger.success(
            f"All {len(tcmd_list)} telecommand function names match their registration names."
        )
    else:
        logger.error(
            "Please ensure that the telecommand function name names match the name registered "
            "in the `telecommand_definitions.c` table."
        )
        sys.exit(1)


if __name__ == "__main__":
    check_tcmd_struct_fields()
    check_tcmd_arg_lists()
    check_tcmd_function_names_match_registration_names()
