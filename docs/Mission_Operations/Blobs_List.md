# Blobs List

## Background

Blobs are small executable payloads (generally 100-5000 bytes), which are programmed in C code, compiled using special techniques, and uplinked to the satellite.

Blobs are used to overcome bugs and add functionality beyond FrontierSat's core firmware when launched.

Running a blob is effectively the same as running a single built-in telecommand, though some blobs are self-rescheduling (and thus run on repeat many times).

Blobs are executed using the `CTS1+exec_blob_from_fs(blob_file,0,arguments_passed_to_blob)` core telecommand. The `0` argument is insignificant in normal use, and should just be 0.

The following is the list of blobs which are ready-to-use on FrontierSat.

## `blobs/hello_world_v1.blob`

Basic, tiny proof-of-concept blob which simply prints back `Hello world from blob`.

This function takes no arguments.

### Example Usage

```
CTS1+exec_blob_from_fs(blobs/hello_world_v1.blob,0,any_value_here)!
```

## `blobs/copy_file_v1.blob`

Blob for copying part of a file into a new file.

The use case for this blob isn't necessarily too large, but it may be useful if we, at some point, want to save space on the filesystem on only keep a small portion of a file available.

### Description

```c
// This is a blob (executable) that will copy a file from one LittleFS file to another.
// It is similar to the "dd" command on unix.
//
// Args Format: <in_path>;<out_path>;<start_offset>;<byte_count>
// The start_offset and byte_count can both be zero to copy the whole file.
```

### Example Usage

The following example copies bytes 100 to 250 (length: 150 bytes) from an MPI data file into a new file.

Take note of the blob arguments being separated by semicolons (`;`) instead of commas.

```
CTS1+exec_blob_from_fs(blobs/copy_file_v1.blob,0,mpi_data/2026-07-01_mpi.dat;mpi_data/2026-07-01_mpi_smaller_output.dat;100;150)!
```

## `blobs/bulk_downlink_start_v2.blob`

Blob to replace the [buggy](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653) `CTS1+comms_bulk_file_downlink_start(<filename>,<start>,<length>)!` command ("v1").

### Description

This blob is nearly a drop-in replacement for the `bulk_file_downlink_start` telecommand:

```c
// This is a blob (executable) that replaces the "CTS1+bulk_file_downlink_start" command.
//
// Motivation: The existing FrontierSat bulk file downlink system contains a bug where you can only
// use it 40 times before the satellite needs a reboot to continue using the filesystem.
// This blob is a workaround to fix that bug/limitation.
//
// Full description of bug: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653
//
// Args Format: <file_path_to_read>;<start_offset>;<byte_count>
// The start_offset and byte_count can both be zero to downlink up to 1 MB.
//
// Usage Example:
// After uplinking the blob as "blobs/bulk_downlink_start_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/bulk_downlink_start_v2.blob,0,your_file.run;0;0)!
```

This blob contains the following benefits above the existing `comms_bulk_file_downlink_start` telecommand:
1. This blob's telecommand response string now includes the filename, file size, and file hash, making it simpler to correlate bulk downlink data with the file it came from, especially when scheduled.
2. Bug is fixed - downlink as many files as many times as you want!

### Example Usage

Assume there exists a file `adcs_data/your_file.run` which you want to bulk downlink.

```
# Previously, you would have ran:
CTS1+comms_bulk_file_downlink_start(adcs_data/your_file.run,0,0)@tsexec=123456@tssent=789!

# Instead though, now you'll run:
CTS1+exec_blob_from_fs(blobs/bulk_downlink_start_v2.blob,0,adcs_data/your_file.run;0;0)@tsexec=123456@tssent=789!
```

## `blobs/extended_beacon_v{2,3,4}.blob`

### Description

Running this blob triggers the extended beacon.

```c
// This is a blob (executable) that emits extended beacons with many extra peripheral fields.
//
// Motivation: The existing FrontierSat beacon is great, but lacks certain data (e.g., ADCS data
// and per-channel EPS data especially).
// This blob is a new feature that allows for sending additional data in the beacon packets.
//
// Args Format: repeat_interval_ms
// The repeat_interval_ms can be 0 to run only once, or any positive number to run repeatedly at
// that specified interval.
//
// Usage Example:
// After uplinking the blob as "blobs/extended_beacon_v4.blob", run:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v4.blob,0,9000)!
```

### Notes

1. Always use "0" as the second argument (i.e., always run with malloc).
2. If the ADCS fails to respond to the OBC, this blob hits the watchdog and crashes because
    the ADCS communications each take about 3.5 seconds to time out. Thus, this blob cannot be
    tested on a dev kit, and must be tested on the flatsat with the ADCS engg model computer.
3. This blob re-schedules itself at the specified interval. Each new scheduled telecommand gets
    a tssent value of `<interval_ms>` after the beacon is sent.
4. If this blob is currently running in repeat mode, and you re-run it, it will first cancel
    the existing repeat telecommand, and then re-schedule itself. That is, it is fine to send
    a command to run this blob on every uplink pass, whether or not it's already running.
5. To stop the recurring rescheduling of this blob after starting it, you can use reboot, or
    use `CTS1+agenda_delete_by_name(exec_blob_from_fs)`, or `CTS1+agenda_delete_all()`, or
    `CTS1+exec_blob_from_fs(blobs/extended_beacon_v4.blob,0,0)!` (which will run one last time,
    then cancel itself).

### Example Usage

To start the extended beacon, repeating every 9 seconds, run:

```
CTS1+exec_blob_from_fs(blobs/extended_beacon_v4.blob,0,9000)!
```

### Versions

* v2
    * Worked well. First extended blob.
    * Has version string ` X2\0`
    * Has packet ID `COMMS_PACKET_TYPE_BEACON_EXTENDED = 0x20`
* v3
    * Tiny upgrade to v2. No changes to the format.
    * Has version string ` X3\0`
    * Has packet ID `COMMS_PACKET_TYPE_BEACON_EXTENDED = 0x20`
    * Change 1: Disable the log message with the args_str. Changed to log level DEBUG.
    * Change 2: Don't increment the `total_tcmd_queued_count` counter (which is downlinked in all beacon packets) when enqueing the blob to re-run.
* v4
    * Tiny upgrade to v3. No changes to the format.
    * Has version string ` X4\0`
    * Has packet ID `COMMS_PACKET_TYPE_BEACON_EXTENDED = 0x20`
    * Change 1 (fix): MPI temperature fetching now supports negative Celsius temperatures.
    * Change 2: Prevent bouncing in the `pending_queued_tcmd_count` field between extended and basic beacons.
    * Improvement: Use error enum for error reporting.

## `blobs/adcs_get_latest_sd_file_v{1,2}.blob`

Running this blobs transfers a file from the ADCS SD card into the LFS `ADCS/` folder, and then bulk downlinks it.

```c
// This is a blob (executable) that will nearly double the rate of ADCS commissioning.
//
// Motivation: Each ADCS commissioning step requires collecting data into an SD file, downlinking
// the list of files, selecting the right file (by its checksum), and then bulk downlinking it. It
// requires two uplink overpasses to get the file. This blob makes it so a single commissioning
// step requires only one uplink overpass to fetch the file.
//
// Args Format: 0 (placeholder, not used)
```

### Description of Blob (Steps)

1. Sets the ADCS SD logging config to stop primary logging (in case it wasn't stopped yet).
2. Walks the ADCS SD card's file list, keeping the pointer at the last (highest-index) entry.
3. Checks if that file is already downloaded/transfered into the `ADCS/` directory. If it is
    not yet downloaded, it downloads it from SD card into LittleFS. Otherwise, it does nothing.
4. Starts the bulk downlink process to download the file.
5. Sends a telecommand response with the file name, size, SHA256 hash, crc16, and file date.

### Notes

1. You should stop the ADCS SD logging before running this command.

### Example Usage

To transfer and downlink the latest file on the ADCS SD card, run:

```
CTS1+exec_blob_from_fs(blobs/adcs_get_latest_sd_file_v2.blob,0,0)!
```

### Versions

* v1
    * Generally worked well. Huge improvement. Had some bugs though.
    * Only reliable on SD cards with roughly 32 or fewer files. Beyond that, the OBC would reboot
        partway through the command.
    * Sometimes downlinked a file other than the latest one, or reported an empty file list, even
        when the file you wanted was there. Re-running it would often pick a different file.
* v2
    * Same arguments and same response format as v1. Use it exactly the same way.
    * Improvement: Handles way more files on the ADCS SD card (>100, probably). Runs way faster.
    * Fix: Always downlinks the newest/latest file, or fails outright. Never downlinks older files.
    * Feature: Response JSON adds `datetime` (the file's timestamp, from the ADCS).
    * Feature: Better error codes/names via an error enum.
    * Fix: No longer pets the watchdog too soon after the firmware's own pets during the file
        transfer, which tripped the IWDG's window (low-side) and rebooted the OBC.
    * Fix: A file that's already in `ADCS/` is only reused if its size matches the size the ADCS
        reports; a partial file left by an interrupted run is re-transferred instead of downlinked.

## `blobs/gnss_bestxyzb_ring_v1.blob`

* Available since: 2026-08-13
* Entirely superseded by `blobs/gnss_bestxyzb_ring_v2.blob`, which contains a very-different implementation.

<details>

### Description

```c
// This is a blob (executable) that periodically requests a "log bestxyzb once" sample from the
// GNSS receiver, stores it into a persistent fixed-size in-memory ring buffer, downlinks
// some random samples from that buffer on every run, and schedules itself for the next run.
//
// Motivation: Collect a rolling history of GNSS position/velocity samples in RAM, and slowly send
// it down to the ground over many passes via random sampling, without needing a dedicated file.
//
// Args Format: <repeat_interval_ms>;<downlink_n>
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   interval (clamped to a minimum of 1100ms).
// - downlink_n: Number of randomly-selected samples to downlink from the ring buffer.
```


### Example Usage

After uplinking the blob as "blobs/gnss_bestxyzb_ring_v1.blob", run:

```
CTS1+eps_set_channel_enabled(gnss,1)!
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v1.blob,0,9000;5)!
```

### Notes

1. Always use "0" as the second argument to exec_blob_from_fs (i.e., always run with malloc).
2. This blob re-schedules itself at the specified interval, same mechanism/caveats as the
   extended beacon blob (re-uplinking cancels any previously-scheduled rerun of this blob).
3. SAFETY FEATURE: On each run, this blob checks whether the GNSS EPS power channel
   (EPS_CHANNEL_3V3_GNSS) is enabled. If the GNSS power is disabled (e.g., EPS safety mode,
   forgot to enable it before, or intentionally disabled it), this blob ends and does not
   reschedule itself.
4. If a GNSS query fails for any reason, this run skips storing a new sample but still
   downlinks existing samples and reschedules normally, so transient GNSS comms errors
   "self-heal" on the next run.
5. If GNSS firehose mode is activated, this blob skips collecting data samples while firehose
   mode is active, but will resume after firehose mode is disabled.
6. The ring buffer's contents may be retained between software reboots, watchdog resets, etc.

</details>

## `blobs/gnss_bestxyzb_ring_v2.blob`

* Available since: 2026-09-20
* Supersedes: `blobs/gnss_bestxyzb_ring_v1.blob`

### High-Level Overview

1. Collect a long rolling history of GNSS position/velocity fixes.
2. Collect GNSS history during MPI operations (with the `TRACK_MPI` flag).
3. Duty-cycle the GNSS receiver so it only draws power when the satellite can afford it.
4. Slowly send that history to the ground over many random SatNOGS passes via random sampling.
5. When the GNSS is active, set the OBC, EPS, and ADCS time based on the GNSS time.


### Description

```c
// This is a blob (executable) that manages the GNSS receiver's power channel based on available
// power/sun, periodically samples "log bestxyzb once" from the GNSS receiver, stores the fixes
// into a ring of files in the LittleFS filesystem, periodically syncs the OBC clock to GNSS time
// (and pushes that time out to the EPS and the ADCS),
// downlinks a randomly-selected consecutive run of stored samples on every run, and schedules
// itself for the next run.
//
// Args Format: <repeat_interval_ms>;<downlink_n>[;<flag1|flag2|...>]
// - repeat_interval_ms: Interval at which the blob re-runs itself. Anything below 1100ms
//   (including 0) is clamped up to 1100ms.
// - downlink_n: Number of ADDITIONAL consecutive samples to downlink after the randomly-selected
//   starting sample. So a total of (1 + downlink_n) packets are sent per run, fewer if the
//   randomly-chosen start lands near the end of the chosen file.
// - flags: Optional. Vertical-bar-separated keywords, matched case-insensitively:
//     STOP       Cancel all pending reruns, turn the GNSS channel off (unless NO_EPS_CTRL), reset the
//                ring, and exit.
//     FAKE       Local/bench test mode: never touch the GNSS UART, never run the power policy
//                or a time sync, and synthesize samples from the hardware RNG instead. Everything
//                else (LittleFS storage, downlink, rescheduling) behaves normally.
//     TRACK_MPI  Additionally force the GNSS on whenever the MPI is in active (sensing) mode,
//                regardless of sun/voltage -- except that the 14V hard floor still wins.
//                This flag only governs powering the GNSS *on*. Downlink is suppressed during
//                MPI activity either way.
//     NO_EPS_CTRL  Never command the EPS channel on or off; just sample if the channel happens to
//                already be on. For bench use and for handing power control back to EPS telecommands.
//                The EPS is still queried every run (PDU housekeeping, for the channel state), and
//                the EPS clock is still set on every time sync.
//     GOOD_ONLY  Only store good, non-empty fixes (see the note on fix filtering). Without it,
//                every BESTXYZB record the receiver returns is stored and downlinked.
```

### Power Policy

Evaluated fresh on every run, in this order; first match wins:

| # | Condition | Action |
|---|-----------|--------|
| 1 | battery < 14000mV | **OFF** (hard floor; overrides everything below) |
| 2 | `TRACK_MPI` set AND MPI is sensing | ON |
| 3 | battery >= 15000mV AND in sun | ON |
| 4 | eclipse OR battery < 14500mV | OFF |
| 5 | otherwise | keep current channel state (hysteresis) |

"In sun" means the sum of coarse sun sensors 1..6 is > 50. If the ADCS query fails, we
conservatively treat it as eclipse (except on the bench, where `RBF=BENCH` steamrolls).
Likewise, if the EPS query for the GNSS channel state fails, the GNSS is treated as off and not
sampled that run (on the bench, `RBF=BENCH` treats it as on).

Battery voltage comes from `OBC_read_vbat_with_adc_mV()`.

### Storage

Samples are stored in LittleFS under `gnss_ring/`, as a ring of 10 files (`gnss_ring/r0.bin` ..
`gnss_ring/r9.bin`), each holding up to 50 fixed-size 144-byte records. When the current file
fills, the blob advances to the next file index; after the last one it wraps back to index 0 and
truncates it, evicting the oldest data. By default, every BESTXYZB record the receiver returns is
stored. With the `GOOD_ONLY` flag, only good, non-empty fixes are stored (solution status must be
`SOL_COMPUTED`, position type must not be `NONE`, and the X/Y/Z position bytes must not be
all-zero).

### Example Usage

After uplinking the blob as "blobs/gnss_bestxyzb_ring_v2.blob", run:

```
# Default:
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,9000;5)!

# Also, log GNSS when the MPI is active:
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,9000;5;TRACK_MPI)!

# Stop/disable this feature, and turn the GNSS off:
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,0;0;STOP)!
```

### Notes
1. Always use "0" as the second argument to exec_blob_from_fs (i.e., always run with malloc).
2. This blob re-schedules itself at the specified interval, same mechanism/caveats as the
   extended beacon blob (re-uplinking cancels any previously-scheduled rerun of this blob).
3. Unlike v1, this blob TURNS THE GNSS CHANNEL ON AND OFF ITSELF (see Power Policy below).
   It no longer dies when it finds the GNSS powered off -- that's now an expected state.
4. SAFETY: The GNSS is never powered on while the battery is below
   GNSS_POWER_HARD_FLOOR_MV (14000mV), and is actively turned off if the battery falls below
   that, regardless of MPI state or sun.
5. SAFETY: Any failure exit which results in non-rescheduling turns the GNSS channel off first,
   unless NO_EPS_CTRL is given.
6. If a GNSS query fails for any reason, this run skips storing a new sample but still
   downlinks existing samples and reschedules normally, so transient GNSS comms errors
   "self-heal" on the next run.
7. If GNSS firehose mode is activated, this blob skips collecting data samples (and time
   syncs) while firehose mode is active, but will resume after GNSS firehose mode is disabled.
8. Fix filtering: by default, every BESTXYZB record the receiver returns is stored and
   downlinked, including warm-up and no-solution records, so the ground sees exactly what the
   receiver reported. With GOOD_ONLY, only good, non-empty fixes are stored: the solution status
   must be SOL_COMPUTED, the position type must not be NONE, and the X/Y/Z position bytes must
   not be all-zero. The response's `bad_fixes` counter only counts records dropped by GOOD_ONLY.
9. While the MPI is in active (sensing) mode, this blob still samples and stores to disk, but
   sends NOTHING over the radio that run, so it doesn't compete with the science campaign.
   This applies whether or not the TRACK_MPI flag was passed.
10. Likewise, no GNSS time sync (OBC clock set) is performed while the MPI is in active
    (sensing) mode, even if one is due: stepping the clock mid-campaign could corrupt the
    timestamps on the science data. The sync stays due and happens on the first run after the
    MPI goes idle. Also not gated behind the TRACK_MPI flag.
11. The GNSS time sync executes when the GNSS is on and sampled, at most once every
    GNSS_TIME_SYNC_INTERVAL_MS (10 minutes).
    It's skipped in FAKE mode, in firehose mode, and while the MPI is active. Every
    successful sync is pushed onward to the EPS (EPS_set_eps_time_based_on_obc_time()) and to
    the ADCS (ADCS_synchronize_unix_time()), even under NO_EPS_CTRL.
12. On a STOP, reset, or serious anomaly, the blob's state is reset, and the collected samples
    are discarded.
13. Nothing is downlinked until the first ring file fills (GNSS_RING_RECORDS_PER_FILE stored
    fixes), because the file being written is never downlinked. Thus, after any reset,
    runs report "sent=0/0" until then.
14. A run that switches the GNSS channel on doesn't sample: the receiver gets
    GNSS_POWER_ON_SETTLE_MS (5s) to boot, and sampling starts on the next run. There's no
    warm-up wait if the channel was already on (e.g., switched on by the ground, or under NO_EPS_CTRL).
15. Return codes: SEND_PARTIAL_FAILURE and SEND_SKIPPED_MPI_ACTIVE are non-zero, but both mean
    the run completed and rescheduled itself as normal. They are not failures of the campaign.

## `blobs/get_file_map_v1.blob`

Blob to get a "map" of a file (hashes, null-byte ranges, and per-chunk CRC16s), to find which parts of a partially-downlinked or partially-uplinked file are wrong or empty.

### Description

```c
// Args Format: <file_path>  or  <file_path>;kwarg1=val;kwarg2=val
// Supported kwargs:
//  - minimum_null_length: Minimum length of a run of 0x00 bytes to be reported. Default: 40.
//  - crc16_chunk_size: Size, in bytes, of each chunk in the crc16_map. Default: file size split
//      into 16 chunks (rounded up). Max of 64 chunks.
```

The response is tightly-packet JSON, like:

```json
{
    "action": "get_file_map_v1",
    "file": "t1.bin",
    "sha256": "4f2d...ae91",
    "crc16": "0xee52",
    "size": 4794,
    "null_ranges": [
        [100,140],[194,1194],[4194,4794]
    ],
    "null_range_count": 3,
    "crc16_map": {
        "0":"0xaaa2","300":"0x0000",...,"4500":"0x0000"
    }
}
```

### Example Usage

```
CTS1+exec_blob_from_fs(blobs/get_file_map_v1.blob,0,your_file.bin)!
CTS1+exec_blob_from_fs(blobs/get_file_map_v1.blob,0,your_file.bin;minimum_null_length=100;crc16_chunk_size=4096)!
```

### Notes

1. Null ranges are `[start, end)` with an exclusive end, like a Python slice (`data[start:end]`).
    Each `crc16_map` key is a chunk's start offset; the chunk runs until the next key's offset (or the end of the file).
2. The CRC16 is the same algorithm as the ADCS file CRC16 (ADCS Firmware ICD `CRC_Calc()`).
3. If the response doesn't fit in the response buffer, it's silently cut off (incomplete JSON). Re-run with a larger `minimum_null_length` to fit fewer null ranges.


## `blobs/analyze_mpi_data_v1.blob`

Blob to summarize an MPI science data file on-orbit (hash, frame count, time syncs, and date range), to decide whether it's worth downlinking.

### Description

```c
// Args Format: <file_path>
```

The response is tightly-packed JSON, like:

```json
{
    "action": "analyze_mpi_data_v1",
    "file": "mpi_data/2026-07-01_mpi.dat",
    "sha256": "6019...b725",
    "size": 39706,
    "frame_count": 245,
    "time_sync_count": 4,
    "malformed_time_sync_count": 0,
    "earliest": {"timestamp_ms": 1782909290000, "datetime": "2026-07-01T123450.000Z_G"},
    "latest": {"timestamp_ms": 1782909299999, "datetime": "2026-07-01T123459.999Z_G"}
}
```

### Example Usage

```
CTS1+exec_blob_from_fs(blobs/analyze_mpi_data_v1.blob,0,mpi_data/your_file.dat)!
```

### Notes

1. The frame count is the number of MPI sync words (`0x0C 0xFF 0xFF 0x0C`) in the file.
2. A "time sync" is the `{"uptime_ms":...,"timestamp_ms":...}` JSON object the firmware writes after each MPI buffer.
    `earliest`/`latest` are the time syncs with the smallest/largest `timestamp_ms` (not necessarily the first/last in the file), or `null` if there are none.
3. A time sync is counted as malformed if it's over 200 bytes long, or lacks a valid `timestamp_ms`.
