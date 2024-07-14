"""Module for generating telecommand previews, as intended to be transmitted."""

import time


def generate_telecommand_preview(
    *,
    tcmd_name: str,
    arg_list: list[str],
    enable_tssent_suffix: bool,
    tsexec_suffix_value: int | str | None = None,
    extra_suffix_tags: dict[str, str] | None = None,
) -> str:
    """Construct a telecommand preview, as intented to be transmitted.

    Args:
    ----
        satellite_prefix: The prefix to use for the telecommand. Default is "CTS1+".
        tcmd_name: The name of the telecommand.
        arg_list: A list of arguments to pass to the telecommand (sent in parenthesis).
        enable_tssent_suffix: Whether to include the "tssent" suffix tag.
        tsexec_suffix_value: The value to use for the "tsexec" suffix tag. 0 to execute
            immediately, per the protocol.
        extra_suffix_tags: A dictionary of extra suffix tags to include in the telecommand.


    Returns:
    -------
        str: The telecommand preview string, as intended to be transmitted.
        Example: "CTS1+hello_world()@tssent=1234!"

    """
    satellite_prefix: str = "CTS1+"

    arg_str = ",".join(arg_list)

    suffix_tags = {}.copy()

    if enable_tssent_suffix:
        suffix_tags["tssent"] = str(int(round(time.time() * 1000)))

    if tsexec_suffix_value is not None:
        if isinstance(tsexec_suffix_value, float):
            msg = "tsexec_suffix_value must be an integer or string."
            raise ValueError(msg)
        suffix_tags["tsexec"] = tsexec_suffix_value

    # Use extra_suffix_tags to override the rest of the suffix tags.
    if extra_suffix_tags is not None:
        suffix_tags.update(extra_suffix_tags)

    # TODO: add the checksum/HMAC suffix here

    suffix_str = "".join([f"@{key}={value}" for key, value in suffix_tags.items()])
    return f"{satellite_prefix}{tcmd_name}({arg_str}){suffix_str}!"
