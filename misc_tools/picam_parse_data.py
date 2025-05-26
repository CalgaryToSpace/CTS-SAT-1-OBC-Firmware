import glob
from loguru import logger
import binascii


def remove_todo_and_colon_lines(input_file, output_file):
    """
    - Removes 54 characters starting from each 'TODO' occurrence.
    - Removes entire lines containing ':' unless they also contain 'TODO'.
    """
    try:
        with open(input_file, "r", encoding="utf-8") as file:
            content = file.read()

        result = []
        index = 0
        while index < len(content):
            todo_pos = content.find("TODO", index)
            if todo_pos == -1:
                remaining = content[index:]
                # Process remaining content line by line
                for line in remaining.splitlines(keepends=True):
                    if ":" in line and "TODO" not in line:
                        continue  # Skip lines with ':' and without TODO
                    result.append(line)
                break
            else:
                # Process lines before TODO for ':'
                segment = content[index:todo_pos]
                for line in segment.splitlines(keepends=True):
                    if ":" in line and "TODO" not in line:
                        continue
                    result.append(line)

                # Skip the 54 characters starting from TODO
                index = todo_pos + 54

        with open(output_file, "w", encoding="utf-8") as file:
            file.write("".join(result))

        print(f"Successfully processed {input_file}. Result saved to {output_file}")

    except Exception as e:
        print(f"An error occurred: {str(e)}")


def merge_non_at_lines(input_file, output_file):
    """
    Removes empty lines and appends lines not starting with '@'
    to the previous '@'-starting line.
    """
    try:
        with open(input_file, "r", encoding="utf-8") as infile:
            lines = infile.readlines()

        merged_lines = []
        current_line = ""

        for line in lines:
            stripped = line.strip()
            if not stripped:
                continue  # Skip empty lines

            if stripped.startswith("@"):
                if current_line:
                    merged_lines.append(current_line + "\n")
                current_line = stripped
            else:
                current_line += stripped  # Append continuation (no space or newline)

        # Append the final line if exists
        if current_line:
            merged_lines.append(current_line + "\n")

        with open(output_file, "w", encoding="utf-8") as outfile:
            outfile.writelines(merged_lines)

        print(f"Processed {input_file} and saved result to {output_file}.")

    except Exception as e:
        print(f"An error occurred: {str(e)}")


def read_and_parse_image(filename, filename_out):
    with open(filename) as fp, open(filename_out, "wb") as fp_out:
        line_num = 0
        while l := fp.readline():
            line_num += 1

            if not l.startswith("@"):
                logger.error(f"Line {line_num} doesn't start with an @ sign. Skipping.")
                continue

            assert len(l) == 66

            l = l[1:]  # remove '@' sign

            line_data = {
                "sentence_num_hex": l[0:4],
                "total_sentences_hex": l[4:8],
                "img_data_hex": l[8:].strip(),
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


if __name__ == "__main__":
    # Clean the input file
    input_file = "05-12-d4.txt"  # Name of txt file with data
    mid_file = "temp.txt"  # Irrelevant name
    output_file = "05-12-d4.txt"  # irrelevant name
    img_file = "05-12-d4.jpg"  # Name of jpg output file

    # Example cleaning process (cleans up the umbilical data, at the time this was created)
    remove_todo_and_colon_lines(input_file, mid_file)
    merge_non_at_lines(mid_file, output_file)

    # Parse the image data (actual image)
    filename = glob.glob(output_file)[0]
    filename_out = img_file
    read_and_parse_image(filename, filename_out)
