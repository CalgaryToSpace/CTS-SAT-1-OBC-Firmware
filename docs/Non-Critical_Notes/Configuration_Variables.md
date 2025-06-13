# Configuration Variables

The satellite supports adding generic uint32_t configuration variables that exist in the OBC's memory.

They can be updated with the `config_set_int_var` telecommand, and fetched with the `config_get_int_var` telecommand.

These variables can be used anywhere in the code (e.g., to configure periods of autonomous actions).

## Adding a configuration variable

1. Create a global extern variable.
2. Add it to the `CONFIG_int_config_variables[]` array in `config/configuration.c`.
3. Use the variable somewhere in the code.
4. Add a proper docstring to the variable in the place its default value is set (where it is defined).

## Additional Notes

String configuration variables are also implemented, but have not been used extensively.
