"""Convert an image in the PiCAM ASCII format to a JPG file.

Run with:

```bash
uv run misc_tools/picam_parse_data.py <input_text_file> <output_jpg_file>
```

"""

# /// script
# dependencies = [
#   "loguru",
# ]
# ///

import binascii
import sys

from loguru import logger


def read_and_parse_image(text_file_in, jpg_file_out):
    with open(text_file_in) as fp, open(jpg_file_out, "wb") as fp_out:
        line_num = 0
        corrupted_lines = 0
        previous_sentence_num = None

        while raw_line := fp.readline():
            line_num += 1
            line = raw_line.strip()

            if not line.startswith("@"):
                logger.error(f"Line {line_num} doesn't start with an @ sign. Skipping.")
                corrupted_lines += 1
                continue

            if len(line) != 65:
                logger.warning(f"Line {line_num} is wrong length ({len(line)} chars). Skipping.")
                corrupted_lines += 1
                continue

            line = line[1:]  # remove '@' sign

            line_data = {
                "sentence_num_hex": line[0:4],
                "total_sentences_hex": line[4:8],
                "img_data_hex": line[8:].strip(),
            }

            # Parse sentence metadata
            try:
                line_data["sentence_num"] = int(line_data["sentence_num_hex"], 16)
            except ValueError:
                corrupted_lines += 1

                logger.error(
                    f"Line {line_num} contains invalid sentence number. "
                    f"sentence_num_hex={line_data['sentence_num_hex']!r}. "
                    f"Attempting to recover from sequence order."
                )

                if previous_sentence_num is None:
                    logger.error(f"Cannot recover sentence number on first line. Skipping.")
                    continue

                line_data["sentence_num"] = previous_sentence_num + 1
                line_data["sentence_num_hex"] = f"{line_data['sentence_num']:04X}"

            try:
                line_data["total_sentences"] = int(line_data["total_sentences_hex"], 16)
            except ValueError as e:
                corrupted_lines += 1

                logger.error(
                    f"Line {line_num} contains invalid total sentence count. "
                    f"total_sentences_hex={line_data['total_sentences_hex']!r}. "
                    f"Skipping line. "
                    f"error={e}"
                )

                continue

            logger.info(f"Line {line_num}: {line_data}")

            if line_data["sentence_num_hex"].upper() == "FACE":
                logger.info(f"Reached end telemetry seq on Line {line_num}.")
                # FIXME: Optionally parse this line per the datasheet's spec.
                continue

            # Decode image data while preserving the expected byte count.
            image_data_hex = line_data["img_data_hex"]

            if len(image_data_hex) != 56:
                corrupted_lines += 1

                logger.error(
                    f"Line {line_num} contains an unexpected amount of image data. "
                    f"Expected 56 hex characters, got {len(image_data_hex)}. "
                    f"Skipping line."
                )

                continue

            try:
                line_img_data = binascii.unhexlify(image_data_hex)
            except (binascii.Error, ValueError) as e:
                corrupted_lines += 1

                logger.error(
                    f"Line {line_num} contains corrupted image data. "
                    f"Sentence {line_data['sentence_num']} contains invalid "
                    f"hex characters. "
                    f"error={e}"
                )

                cleaned_image_data_hex = ""

                for char in image_data_hex:
                    if char in "0123456789abcdefABCDEF":
                        cleaned_image_data_hex += char
                    else:
                        cleaned_image_data_hex += "0"

                line_img_data = binascii.unhexlify(cleaned_image_data_hex)

                logger.warning(
                    f"Recovered Sentence {line_data['sentence_num']} "
                    f"using zero substitution for invalid hex characters."
                )

            fp_out.write(line_img_data)

            previous_sentence_num = line_data["sentence_num"]

        logger.info(f"Finished parsing image. {corrupted_lines} corrupted lines encountered.")


def main():
    print("Usage: python picam_parse_data.py <input_text_file> <output_jpg_file>")
    if len(sys.argv) != 3:
        print("Error: Invalid number of arguments.")
        main()
        sys.exit(1)
    text_file_in = sys.argv[1]
    jpg_file_out = sys.argv[2]

    if not jpg_file_out.endswith(".jpg"):
        print("Error: Output file must be a .jpg file.")
        sys.exit(1)

    read_and_parse_image(text_file_in, jpg_file_out)


if __name__ == "__main__":
    main()
    print("Done.")
