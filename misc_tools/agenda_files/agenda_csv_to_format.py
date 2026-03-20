# /// script
# dependencies = [
#   "polars~=1.39",
#   "tyro==1.0.10",
#   "loguru",
# ]
# ///

from pathlib import Path
import hashlib

import polars as pl
from loguru import logger
import tyro


def main(input_csv_file: Path, *, output_agenda_file: Path | None = None) -> None:
    if output_agenda_file is None:
        output_agenda_file = input_csv_file.with_name(input_csv_file.stem + "_agenda.txt")

    df = pl.read_csv(input_csv_file, infer_schema=False)
    df = df.select(
        "tsexec_date",
        "tsexec_time",
        "command",
        "resp_fname",
    ).with_columns(
        pl.all().str.strip_chars().replace({"": None}),
    )
    logger.info(f"Loaded file: {df.height} rows")

    df = df.with_columns(
        tsexec_date=pl.col("tsexec_date").str.to_date("%Y-%m-%d"),
        tsexec_time=pl.col("tsexec_time").str.to_time("%H:%M:%S"),
    )

    df = (
        df.with_columns(
            tsexec_ms=(
                pl.col("tsexec_date")
                .dt.combine(pl.col("tsexec_time"))
                .dt.epoch("ms")
                .cast(pl.Int64)
            ),
        )
        .sort("tsexec_ms")  # Sorting might not actually be critical, but good idea.
        .with_columns(
            # Copy tsexec to tssent for duplicate detection.
            tssent_ms=pl.col("tsexec_ms"),
        )
        .with_columns(
            output=(
                pl.format(
                    "CTS1+{}@tssent={}@tsexec={}{}!",
                    pl.col("command"),
                    pl.col("tssent_ms"),
                    pl.col("tsexec_ms"),
                    # Optional resp_fname suffix tag:
                    pl.when(pl.col("resp_fname").is_not_null())
                    .then(pl.lit("@resp_fname=") + pl.col("resp_fname"))
                    .otherwise(pl.lit("")),
                )
            )
        )
    )

    # Use write_bytes to force "\n" newlines on Windows!
    output_bytes = (
        "\n".join(df["output"].to_list())
        # Critial: Must have a trailing newline!
        + "\n"
    ).encode()
    output_agenda_file.write_bytes(output_bytes)

    logger.info(
        f"Wrote output. {len(output_bytes):,} bytes. SHA256: {hashlib.sha256(output_bytes).hexdigest()}"
    )


if __name__ == "__main__":
    tyro.cli(main)
