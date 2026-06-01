# Filesystem Mission Operations

CTS-SAT-1 uses the LittleFS filesystem to store files on the flash memory module(s) on the satellite.

A filesystem allows naming files, deleting files, writing to several files simultaneously, etc.

## LittleFS Error Codes

```c
enum lfs_error {
    LFS_ERR_OK          = 0,    // No error
    LFS_ERR_IO          = -5,   // Error during device operation
    LFS_ERR_CORRUPT     = -84,  // Corrupted
    LFS_ERR_NOENT       = -2,   // No directory entry
    LFS_ERR_EXIST       = -17,  // Entry already exists
    LFS_ERR_NOTDIR      = -20,  // Entry is not a dir
    LFS_ERR_ISDIR       = -21,  // Entry is a dir
    LFS_ERR_NOTEMPTY    = -39,  // Dir is not empty
    LFS_ERR_BADF        = -9,   // Bad file number
    LFS_ERR_FBIG        = -27,  // File too large
    LFS_ERR_INVAL       = -22,  // Invalid parameter
    LFS_ERR_NOSPC       = -28,  // No space left on device
    LFS_ERR_NOMEM       = -12,  // No more memory available
    LFS_ERR_NOATTR      = -61,  // No data/attr available
    LFS_ERR_NAMETOOLONG = -36,  // File name too long
};
```

## General Guidelines

* A file path can only be accessed from one "place" at a time. There exists a risk of corruption and/or undefined behaviour if multiple points in code attempt to access the same file path.
    * For example, do not write to the same file using `@resp_fname=` and then "GNSS firehose storage mode".
    * Avoid risky operations where nominal execution avoids this issue, but any anomaly (e.g., missed uplink, failed execution) may risk double-access.
* The more files in a folder, the slower it is to open a new file in that folder.
    * Try to maintain less than 15-30 entries in each directory.
    * Make subdirectories as you approach the limit (e.g., `gnss/2026-06/2026-06-01/2026-06-01-T01.gnss`, `gnss/2026-06/2026-06-01/2026-06-01-T02.gnss`).

### Conventions

* Well-named files should contain, within the filename, all information that is contained within the names of parent folders. File names should indicate both when the file was created, and roughly what type of data it contains. File names should be standalone, because the folder names are not necessarily maintained after downlinking. For example, `2026-06-01.log` is a bad name, but `2026-06-01_image_capture.img` is a better name.
* Maintaining a list of files in a spreadsheet as they get created may be wise in order to keep track of which files have been downlinked.
