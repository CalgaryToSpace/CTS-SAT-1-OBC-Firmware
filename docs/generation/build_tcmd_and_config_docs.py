"""Generate a Markdown file listing all telecommands and configuration variables.

The telecommand and configuration variable definitions are downloaded from the
CTS-SAT-1-Ground-Support repository's bundled data files (which are themselves
auto-generated from the OBC firmware source).

Run this script with:
```bash
pip install uv

uv run docs/generation/build_tcmd_and_config_list.py
```

The `uv` tool automatically installs the required dependencies specified in the
`dependencies` section below.
"""

# /// script
# dependencies = [
#   "requests>=2.31,<3",
#   "gitpython",
# ]
# ///

from pathlib import Path

import git
import requests

TELECOMMAND_LIST_URL = "https://raw.githubusercontent.com/CalgaryToSpace/CTS-SAT-1-Ground-Support/refs/heads/main/cts1_ground_support/bundled_data/cts_sat_1_telecommand_list.json"
CONFIG_VARIABLE_LIST_URL = "https://raw.githubusercontent.com/CalgaryToSpace/CTS-SAT-1-Ground-Support/refs/heads/main/cts1_ground_support/bundled_data/cts_sat_1_config_variable_list.json"

READINESS_LEVEL_PREFIX = "TCMD_READINESS_LEVEL_"


def download_json(url: str) -> list | dict:
    """Downloads a JSON file from `url` and returns its parsed content."""
    print(f"Downloading: {url}")
    response = requests.get(url, timeout=30)
    response.raise_for_status()
    return response.json()


def strip_readiness_prefix(readiness_level: str) -> str:
    """Strips the verbose `TCMD_READINESS_LEVEL_` prefix for nicer display."""
    if readiness_level.startswith(READINESS_LEVEL_PREFIX):
        return readiness_level[len(READINESS_LEVEL_PREFIX) :]
    return readiness_level


def escape_md_cell(text: str | None) -> str:
    """Makes a string safe to put inside a single Markdown table cell.

    Pipes are escaped, and newlines are replaced with `<br>` so multi-line
    docstrings render as multi-line cells.
    """
    if text is None:
        return ""
    return text.replace("|", "\\|").replace("\r\n", "\n").replace("\n", "<br>")


def render_telecommand_section(telecommands: list[dict]) -> str:
    """Renders the telecommand section of the Markdown file."""

    # Sort alphabetically by name for stable, easy-to-scan output.
    telecommands = sorted(telecommands, key=lambda t: t["name"])

    lines: list[str] = []
    lines.append("# Telecommands")
    lines.append("")
    lines.append(f"There are **{len(telecommands)}** telecommands defined.")
    lines.append("")

    # Readiness level legend.
    lines.append("## Readiness Levels")
    lines.append("")
    lines.append("Telecommand readiness levels indicate the operational risk of a telecommand.")
    lines.append("The `TCMD_READINESS_LEVEL_` prefix is stripped in tables below.")
    lines.append("")
    readiness_counts: dict[str, int] = {}
    for tcmd in telecommands:
        key = strip_readiness_prefix(tcmd["readiness_level"])
        readiness_counts[key] = readiness_counts.get(key, 0) + 1
    lines.append("| Readiness Level | Count |")
    lines.append("| --- | ---: |")
    for level, count in sorted(readiness_counts.items()):
        lines.append(f"| `{level}` | {count} |")
    lines.append("")

    # Quick-reference summary table.
    lines.append("## Summary Table")
    lines.append("")
    lines.append("| # | Name | Args | Readiness Level |")
    lines.append("| ---: | --- | ---: | --- |")
    for idx, tcmd in enumerate(telecommands, start=1):
        name = tcmd["name"]
        n_args = tcmd["number_of_args"]
        level = strip_readiness_prefix(tcmd["readiness_level"])
        lines.append(f"| {idx} | [`{name}`](#tcmd-{name}) | {n_args} | `{level}` |")
    lines.append("")

    # Detail section: one subsection per telecommand.
    lines.append("## Telecommand Details")
    lines.append("")
    for tcmd in telecommands:
        name = tcmd["name"]
        lines.append(f'### <a id="tcmd-{name}"></a>`{name}`')
        lines.append("")

        # Compact metadata table.
        lines.append("| Field | Value |")
        lines.append("| --- | --- |")
        lines.append(f"| Function | `{tcmd['tcmd_func']}` |")
        lines.append(f"| Number of Arguments | {tcmd['number_of_args']} |")
        lines.append(f"| Readiness Level | `{strip_readiness_prefix(tcmd['readiness_level'])}` |")
        if tcmd.get("description"):
            lines.append(f"| Description | {escape_md_cell(tcmd['description'])} |")
        lines.append("")

        # Argument descriptions list (if any).
        arg_descs = tcmd.get("argument_descriptions") or []
        if arg_descs:
            lines.append("**Arguments:**")
            lines.append("")
            for i, desc in enumerate(arg_descs):
                lines.append(f"{i}. {desc}")
            lines.append("")

        # Full docstring as a fenced code block to preserve formatting verbatim.
        docstring = tcmd.get("full_docstring")
        if docstring:
            lines.append("**Full docstring:**")
            lines.append("")
            lines.append("```")
            lines.append(docstring.rstrip())
            lines.append("```")
            lines.append("")

    return "\n".join(lines)


def render_config_variable_section(config_data: dict) -> str:
    """Renders the configuration variables section of the Markdown file."""

    int_vars = sorted(
        config_data.get("int_config_variables", []),
        key=lambda v: v["variable_name"],
    )
    str_vars = sorted(
        config_data.get("str_config_variables", []),
        key=lambda v: v["variable_name"],
    )

    lines: list[str] = []
    lines.append("# Configuration Variables")
    lines.append("")
    lines.append(
        f"There are **{len(int_vars)}** integer and **{len(str_vars)}** string "
        "configuration variables."
    )
    lines.append("")

    # Quick-reference summary table covering both int and str variables.
    lines.append("## Summary Table")
    lines.append("")
    lines.append("| # | Variable Name | Type | Default Value |")
    lines.append("| ---: | --- | --- | --- |")
    idx = 0
    for var in int_vars:
        idx += 1
        name = var["variable_name"]
        default = var["default_value"]
        lines.append(f"| {idx} | [`{name}`](#cfg-{name}) | `int` | `{default}` |")
    for var in str_vars:
        idx += 1
        name = var["variable_name"]
        # Default is a string; escape pipes/newlines for the cell.
        default = escape_md_cell(var["default_value"])
        lines.append(f"| {idx} | [`{name}`](#cfg-{name}) | `str` | `{default}` |")
    lines.append("")

    # Detail section: one subsection per integer config variable.
    lines.append("## Integer Configuration Variable Details")
    lines.append("")
    for var in int_vars:
        name = var["variable_name"]
        lines.append(f'### <a id="cfg-{name}"></a>`{name}`')
        lines.append("")

        # Compact metadata table.
        lines.append("| Field | Value |")
        lines.append("| --- | --- |")
        lines.append("| Type | `int` |")
        lines.append(f"| Default Value | `{var['default_value']}` |")
        lines.append("")

        # Docstring as a fenced code block to preserve formatting verbatim.
        docstring = var.get("docstring")
        if docstring:
            lines.append("**Docstring:**")
            lines.append("")
            lines.append("```")
            lines.append(docstring.rstrip())
            lines.append("```")
            lines.append("")

    # Detail section: one subsection per string config variable.
    lines.append("## String Configuration Variable Details")
    lines.append("")
    for var in str_vars:
        name = var["variable_name"]
        lines.append(f'### <a id="cfg-{name}"></a>`{name}`')
        lines.append("")

        # Compact metadata table.
        lines.append("| Field | Value |")
        lines.append("| --- | --- |")
        lines.append("| Type | `str` |")
        lines.append(f"| Max Length | {var['max_length']} |")
        lines.append(f"| Default Value | `{escape_md_cell(var['default_value'])}` |")
        lines.append("")

        # Docstring as a fenced code block to preserve formatting verbatim.
        docstring = var.get("docstring")
        if docstring:
            lines.append("**Docstring:**")
            lines.append("")
            lines.append("```")
            lines.append(docstring.rstrip())
            lines.append("```")
            lines.append("")

    return "\n".join(lines)


MD_FILE_TEMPLATE = """\
# Telecommands and Configuration Variables

This is an auto-generated reference of all telecommands and configuration
variables supported by the CTS-SAT-1 OBC firmware. It is generated by the
`{this_script_file_name}` script.

Source data downloaded from the
[`CTS-SAT-1-Ground-Support`](https://github.com/CalgaryToSpace/CTS-SAT-1-Ground-Support)
repository's bundled data files.

## Table of Contents

- [Telecommands](#telecommands)
  - [Readiness Levels](#readiness-levels)
  - [Summary Table](#summary-table)
  - [Telecommand Details](#telecommand-details)
- [Configuration Variables](#configuration-variables)
  - [Summary Table](#summary-table-1)
  - [Integer Configuration Variable Details](#integer-configuration-variable-details)
  - [String Configuration Variable Details](#string-configuration-variable-details)

---

{telecommand_section}

---

{config_variable_section}
"""


def generate_markdown_file(output_path: Path) -> None:
    """Downloads source data and writes the assembled Markdown file."""

    telecommands = download_json(TELECOMMAND_LIST_URL)
    config_data = download_json(CONFIG_VARIABLE_LIST_URL)

    assert isinstance(telecommands, list), "Expected telecommand list to be a JSON array."
    assert isinstance(config_data, dict), "Expected config variable list to be a JSON object."

    telecommand_section = render_telecommand_section(telecommands)
    config_variable_section = render_config_variable_section(config_data)

    md_content = MD_FILE_TEMPLATE.format(
        this_script_file_name=Path(__file__).name,
        telecommand_section=telecommand_section,
        config_variable_section=config_variable_section,
    )

    with open(output_path, "w") as f:
        f.write(md_content)


def main() -> None:
    repo_root_path = Path(git.Repo(".", search_parent_directories=True).working_tree_dir)
    output_path = (
        repo_root_path / "docs" / "Mission_Operations" / "Telecommands_and_Config_Variables.md"
    )
    output_path.parent.mkdir(parents=True, exist_ok=True)

    generate_markdown_file(output_path)

    print(f"Generated telecommand and config variable list at: {output_path}")


if __name__ == "__main__":
    main()
