import sys
from loguru import logger
import binascii


def read_and_parse_image(text_file_in, jpg_file_out):
    with open(text_file_in) as fp, open(jpg_file_out, "wb") as fp_out:
        line_num = 0
        while line := fp.readline():
            line_num += 1

            if not line.startswith("@"):
                logger.error(f"Line {line_num} doesn't start with an @ sign. Skipping.")
                continue

            assert len(line) == 66

            line = line[1:]  # remove '@' sign

            line_data = {
                "sentence_num_hex": line[0:4],
                "total_sentences_hex": line[4:8],
                "img_data_hex": line[8:].strip(),
            }

            line_data["sentence_num"] = int(line_data["sentence_num_hex"], 16)
            line_data["total_sentences"] = int(line_data["total_sentences_hex"], 16)

            logger.info(f"Line {line_num}: {line_data}")

            if line_data["sentence_num_hex"] == "FACE":
                logger.info(f"Reached end telemetry seq on Line {line_num}.")
                # FIXME optionally parse this line, per the datasheet's spec
                continue

            line_img_data = binascii.unhexlify(line_data["img_data_hex"])

            fp_out.write(line_img_data)


def main():
    print("Usage: python picam_parse_data.py <input_text_file> <output_jpg_file>")
    if len(sys.argv) != 3:
        print("Error: Invalid number of arguments.")
        main()
        sys.exit(1)
    text_file_in = sys.argv[1]
    jpg_file_out = sys.argv[2]
    if not text_file_in.endswith(".txt"):
        print("Error: Input file must be a .txt file.")
        main()
        sys.exit(1)
    if not jpg_file_out.endswith(".jpg"):
        print("Error: Output file must be a .jpg file.")
        main()
        sys.exit(1)
    read_and_parse_image(text_file_in, jpg_file_out)


if __name__ == "__main__":
    main()
    print("Done.")
