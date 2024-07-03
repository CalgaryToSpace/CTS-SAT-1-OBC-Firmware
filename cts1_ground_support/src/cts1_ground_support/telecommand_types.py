"""Types related to storing telecommand definitions."""

from dataclasses import dataclass


@dataclass(kw_only=True)
class TelecommandDefinition:
    """Stores a telecommand definition. from the `telecommand_definitions.c` file."""

    name: str
    tcmd_func: str
    description: str | None = None
    number_of_args: int

    def to_dict(self: "TelecommandDefinition") -> dict[str, str | int]:
        """Convert the telecommand definition to a dictionary."""
        return {
            "name": self.name,
            "tcmd_func": self.tcmd_func,
            "description": self.description,
            "number_of_args": self.number_of_args,
        }
