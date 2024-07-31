"""Types related to storing telecommand definitions."""

from dataclasses import dataclass


@dataclass(kw_only=True)
class TelecommandDefinition:
    """Stores a telecommand definition. from the `telecommand_definitions.c` file."""

    name: str
    tcmd_func: str
    description: str | None = None
    number_of_args: int
    readiness_level: str
    full_docstring: str | None = None
    argument_descriptions: list[str] | None = None

    def to_dict(self: "TelecommandDefinition") -> dict[str, str | int | list[str] | None]:
        """Convert the telecommand definition to a dictionary."""
        return {
            "name": self.name,
            "tcmd_func": self.tcmd_func,
            "description": self.description,
            "number_of_args": self.number_of_args,
            "readiness_level": self.readiness_level,
            "full_docstring": self.full_docstring,
            "argument_descriptions": self.argument_descriptions,
        }

    def to_dict_table_fields(self: "TelecommandDefinition") -> dict[str, str | int]:
        """Convert the telecommand definition to a dictionary, with only the fields from the array.

        Returns
        -------
            dict[str, str | int]: A dict of the fields in the telecommand registration array.

        """
        return {
            "Command": self.name,
            "Function Name": self.tcmd_func,
            "Number of Args": self.number_of_args,
            "Readiness Level": self.readiness_level,
        }

    def has_required_fields(self: "TelecommandDefinition") -> bool:
        """Check if the telecommand definition has all the required fields."""
        return all(
            [
                self.name is not None,
                self.tcmd_func is not None,
                self.number_of_args is not None,
                self.readiness_level is not None,
            ]
        )
