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

## `blobs/adcs_get_latest_sd_file_v1.blob`

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
2. Lists all files on the ADCS SD card.
3. Determine's the latest file, by index, on the SD card.
4. Checks if that file has "is_busy_updating = true". Returns error code 96 if it does.
5. Checks if that file is already downloaded/transfered into the `ADCS/` directory. If it is
    not yet downloaded, it downloads it. Otherwise, it does nothing.
6. Starts the bulk downlink process to download the file.
7. Sends a telecommand response with the file name, size, hash, and crc16.

### Notes
1. Likely doesn't work if there are more than 70 files on the SD card. It's the way it has to be.
2. You MUST stop the ADCS SD logging before running this command.

### Example Usage

To transfer and downlink the latest file on the ADCS SD card, run:

```
CTS1+exec_blob_from_fs(blobs/adcs_get_latest_sd_file_v1.blob,0,0)!
```

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

### Description

```c
// This is a blob (executable) that manages the GNSS receiver's power channel based on available
// power/sun, periodically samples "log bestxyzb once" from the GNSS receiver, stores good fixes
// into a ring of files in the LittleFS filesystem, periodically syncs the OBC clock to GNSS time
// (and pushes that time out to the EPS and the ADCS),
// downlinks a randomly-selected consecutive run of stored samples on every run, and schedules
// itself for the next run.
//
// Args Format: <repeat_interval_ms>;<downlink_n>[;<flags>]
// - repeat_interval_ms: 0 to run only once, or any positive number to run repeatedly at that
//   interval (clamped to a minimum of 1100ms).
// - downlink_n: Number of ADDITIONAL consecutive samples to downlink after the randomly-selected
//   starting sample. So a total of (1 + downlink_n) packets are sent per run, fewer if the
//   randomly-chosen start lands near the end of the chosen file.
// - flags: Optional. Vertical-bar-separated keywords, matched case-insensitively:
//     STOP       Permanently cancel this blob.
//     RESUME     Clear the persistent stop flag set by STOP, and run normally.
//     FAKE       Bench test mode: no GNSS/EPS access; samples synthesized from the RNG.
//     TRACK_MPI  Additionally force the GNSS on whenever the MPI is in active (sensing) mode.
//                (Only governs powering the GNSS on; downlink is suppressed during MPI
//                activity either way -- see note 9.)
//     NOEPS      Never command the EPS channel on or off.
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

"In sun" means the sum of coarse sun sensors 1..6 is > 100. If the ADCS query fails, we
conservatively treat it as eclipse (except on the bench, where `RBF=BENCH` steamrolls).

Battery voltage comes from `OBC_read_vbat_with_adc_mV()`.

### Storage

Samples are stored in LittleFS under `gnss_ring/`, as a ring of 10 files (`gnss_ring/r0.bin` ..
`gnss_ring/r9.bin`), each holding up to 50 fixed-size 144-byte records. When the current file
fills, the blob advances to the next file index; after the last one it wraps back to index 0 and
truncates it, evicting the oldest data. Only good, non-empty fixes are stored (solution status
must be `SOL_COMPUTED`, position type must not be `NONE`, and the X/Y/Z position bytes must not
be all-zero).

Unlike v1, sample data is NOT kept in RAM. Only a small write cursor / flag / counter block (plus
the handle of the ring file currently being written) lives in the fixed SRAM region. After a power
cycle, that block is NOT rebuilt from disk: the ring simply restarts at `r0.bin` record 0,
truncating each file as it first writes to it.

For speed, the ring file being appended to is left OPEN between the blob's executions, so storing
a sample is a single `lfs_file_write()` rather than an open/write/close cycle. It is closed (which
is what commits its records to flash) when the file fills up, on `STOP`, and on a non-repeating
(one-shot) run. Two consequences: an unexpected reboot loses the records in the file currently
being written (up to 50), and that file is not downlinked while it's open -- its samples start
going down once it fills up and the ring moves on to the next file.

### Example Usage

After uplinking the blob as "blobs/gnss_bestxyzb_ring_v2.blob", run:

```
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,9000;5;TRACK_MPI)!
```

To permanently stop it (also turns the GNSS channel off and cancels all pending reruns):

```
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,0;0;STOP)!
```

To restart it after a STOP:

```
CTS1+exec_blob_from_fs(blobs/gnss_bestxyzb_ring_v2.blob,0,9000;5;RESUME|TRACK_MPI)!
```

### Notes

1. Always use "0" as the second argument to exec_blob_from_fs (i.e., always run with malloc).
2. This blob re-schedules itself at the specified interval, same mechanism/caveats as the
   extended beacon blob (re-uplinking cancels any previously-scheduled rerun of this blob).
3. This blob turns the GNSS EPS channel on and off autonomously.
4. SAFETY: The GNSS is never powered on while the battery is below 14000mV, and is actively
   turned off if the battery falls below that, no matter what.
5. While the GNSS is powered on, the blob re-syncs the OBC clock from GNSS time at most once
   every 10 minutes, and then pushes that time onward to the EPS and the ADCS, so GNSS is the
   authoritative time reference while the receiver is powered. The EPS push is what makes the
   sync stick -- the firmware's background upkeep task treats the EPS RTC as authoritative and
   would otherwise revert the OBC clock to it within `EPS_time_sync_period_sec`. Both pushes are
   best-effort and reported in the response string as `eps_time=`/`adcs_time=` (0 = accepted).
   The `NOEPS` flag does not suppress the EPS time push; it only governs channel power control.
6. If a GNSS query fails for any reason, this run skips storing a new sample but still downlinks
   existing samples and reschedules normally, so transient GNSS comms errors "self-heal".
7. If GNSS firehose mode is activated, this blob skips collecting data samples (and time syncs)
   while firehose mode is active, but will resume after firehose mode is disabled.
8. The STOP flag is latched in the same SRAM region as the write cursor, so it survives software
   reboots but not a full power cycle that clears SRAM.
9. While the MPI is in active (sensing) mode, this blob still samples and stores to disk, but
   sends NOTHING over the radio that run, so it doesn't compete with the science campaign.
   Nothing is lost: the stored samples go down on a later run, once the MPI is idle. This applies
   whether or not the `TRACK_MPI` flag was passed. Such runs return
   `DOWNLINK_SKIPPED_MPI_ACTIVE` (62) and report `sent=skipped(mpi_active)`.
