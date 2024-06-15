"""Helpers for dealing with raw bytes, displaying them, etc."""


def bytes_to_nice_str(byte_obj: bytes, *, newlines_as_hex: bool = True) -> str:
    """Print a byte object as hex or ASCII, whichever is better.

    Example print: [0xDA][0xBE]INFO: boot complete[0xDA][0xED]
    """
    out: str = ""
    for b in byte_obj:
        # if it's a printable ASCII character
        if not newlines_as_hex and b == 0x0A:  # noqa: PLR2004
            out += "[0x0A]\n"
        elif 0x20 <= b <= 0x7E:  # noqa: PLR2004
            out += bytes([b]).decode("ascii")
        else:
            hex_repr = hex(b)  # like '0XA'
            hex_repr = hex_repr.upper().replace("0X", "").zfill(2)
            hex_repr = f"0x{hex_repr}"
            out += f"[{hex_repr}]"
    return out
