"""Helpers for dealing with raw bytes, displaying them, etc."""


def format_byte_as_hex(b: int) -> str:
    """Format a byte as a 2-character hex string.

    Example: 1 -> "[0x01]"
    """
    return f"[0x{hex(b)[2:].upper().zfill(2)}]"


def bytes_to_nice_str(byte_obj: bytes, *, show_end_of_line_chars: bool) -> str:
    """Convert a 'bytes' object to hex and ASCII/UTF-8, whichever is better for each character.

    Example print: [0xDA][0xBE]INFO: boot complete[0xDA][0xED]
    """
    out: str = ""
    for b in byte_obj:
        # Check if it's a printable ASCII character.
        # 'b' is an int.

        if b == 0x0A:  # noqa: PLR2004 (magic number 0x0A=newline)
            if show_end_of_line_chars:
                out += "↴"  # ↴ (DOWNWARDS ARROW WITH CORNER LEFTWARDS) - Unicode: U+21B4
            out += "\n"
        elif b == 0x0D:  # noqa: PLR2004 (magic number 0x0D=carriage return)
            if show_end_of_line_chars:
                out += "↵"  # ↵ (DOWNWARDS ARROW WITH CORNER LEFTWARDS) - Unicode: U+21B5
        elif 0x20 <= b <= 0x7E:  # noqa: PLR2004
            out += bytes([b]).decode("ascii")
        else:
            out += format_byte_as_hex(b)

    # Convert some Unicode characters back to unicode (e.g., for unit tests).
    good_unicode_chars = {"✔️", "✅", "❌"}
    for char in good_unicode_chars:
        char_as_bytes = char.encode("utf-8")
        find_str = "".join(format_byte_as_hex(b) for b in char_as_bytes)
        out = out.replace(find_str, char)

    return out
