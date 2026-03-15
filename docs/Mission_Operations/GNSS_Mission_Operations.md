# GNSS Mission Operations

## Correlating GNSS data to a timestamp

All OEM7 ASCII responses contain the GPS Week and GPS Seconds fields in the header (e.g., the `log bestxyza once` command). Those fields can easily be converted to a timestamp/date.

```python
from datetime import datetime, timedelta, timezone

def gps_to_datetime(gps_week: int, gps_seconds: float):
    gps_epoch = datetime(1980, 1, 6, tzinfo=timezone.utc)
    return gps_epoch + timedelta(weeks=gps_week, seconds=gps_seconds)
```

## Task: Fetch GNSS data once

Query the GNSS for ASCII data in command-mode (simple request-then-single-response).

You may want to use CTS-SAT-1 features like scheduling and storing the command response to a file (`@resp_fname=`).

### Procedure

1. Power on the EPS channel for the GNSS receiver (`gnss`).
2. Optionally, wait 0.5-5 minutes for the GNSS receiver to get a lock.
3. Use the `gnss_send_cmd_ascii` command to query the GNSS: `CTS1+gnss_send_cmd_ascii(log bestxyza once)!`
4. When complete, power off the EPS channel.

## Task: Enabling continuous GNSS data storage to file

A CTS-SAT-1 feature allows an operator to enable period GNSS logging (like `log bestxyza ontime 10`), and store it to a file as the data comes in.

This feature is called "firehose storage" mode (because the data comes somewhat rapidly at unknown points in time), and is in contrast to "command mode".

This feature works alongside MPI data collection.

### Procedure

1. Power on the EPS channel for the GNSS receiver (`gnss`).
2. Optionally, wait 0.5-5 minutes for the GNSS receiver to get a lock.
3. Use `gnss_send_cmd_ascii` to enable the GNSS message types desired: `CTS1+gnss_send_cmd_ascii(log bestxyza ontime 30)!`
4. Enable "firehose storage mode": `CTS1+gnss_enable_firehose_storage_mode(filename)`
5. Nominally, leave data collection running for a long time (e.g., while the satellite passes over the poles).
6. Disable firehose storage mode: `CTS1+gnss_disable_firehose_storage_mode()`
7. Power off the GNSS receiver.
