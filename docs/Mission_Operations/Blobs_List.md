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

Blob to replace the [buggy](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/653) `CTS1+comms_bulk_file_downlink_start(<filename>,<start>,<length>)!` command.

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
