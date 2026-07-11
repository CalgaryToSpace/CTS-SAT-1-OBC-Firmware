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
