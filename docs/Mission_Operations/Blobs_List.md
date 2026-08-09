# Blobs List

## Background

Blobs are small executable payloads (generally 100-5000 bytes), which are programmed in C code, compiled using special techniques, and uplinked to the satellite.

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

## `blobs/extended_beacon_v2.blob`

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
// After uplinking the blob as "blobs/extended_beacon_v2.blob", run:
// CTS1+exec_blob_from_fs(blobs/extended_beacon_v2.blob,0,9000)!
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
    `CTS1+exec_blob_from_fs(blobs/extended_beacon_v2.blob,0,0)!` (which will run one last time,
    then cancel itself).

### Example Usage

To start the extended beacon, repeating every 9 seconds, run:

```
CTS1+exec_blob_from_fs(blobs/extended_beacon_v2.blob,0,9000)!
```

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
