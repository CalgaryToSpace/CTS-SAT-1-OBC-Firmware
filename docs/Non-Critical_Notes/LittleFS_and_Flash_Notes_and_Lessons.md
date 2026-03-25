# LittleFS and Flash Memory - Notes and Lessons

The LittleFS filesystem is used to provide a layer on top of the raw flash memory module (block device) which allows for files/folders, and other standard filesystem features (reading, appending, etc.).

## Flash Memory Module

* Chip: MT29F1G 1Gib (128MiB)
    * NAND FLASH
    * Datasheet: https://www.farnell.com/datasheets/3151163.pdf
    * Sizes:
        * Page Size: 2048 bytes (+ error correction)
        * Block Size: 64 pages = 128 KiB = 131072 bytes
        * Plane (entire chip) Size: 1024 blocks = 128 MiB = 134217728 bytes
        * Chip Size = 2048 bytes per page * 64 pages per block * 1024 blocks per plane
        * You can write one PAGE at a time, but you have to erase a whole BLOCK at a time.

* SPI DMA is faster.
* The MT29F1G chip requires that the CS line be driven to inactive between writes, otherwise the chip/system goes unresponsive.

## LittleFS

* Solid filesystem once everything is implemented right.
* Uses Copy-on-Write. Thus, modifying a file takes a long time as its block(s) get copied.

### LittleFS Configuration

* `metadata_max` is an important configuration variable.
    * Decrease the time metadata compaction operations take (operation is very slow).
    * Required to prevent watchdog trigger loop (i.e., prevents extremely-long writes).
    * See: https://github.com/littlefs-project/littlefs/issues/1079#issuecomment-2720048008
    * Alternative/additional option described in issue: Call `lfs_fs_gc()` periodically during idle.
    * Default value is the entire blocksize. We lower it to: `.metadata_max = 1024 * 8`

### Be Aware

* You must close all files (even read-only ones), or you risk in-SRAM state corruption as pointers to the stack aren't freed.
* Unmounting not actually do anything to the device, and does not lock out filesystem operations.
    * Thus, you have to be confident the filesystem is mounted before attempting file operations.
* The filesystem slows down as more blocks are used/written to.
* The filesystem can only store as many files as there are blocks. That is, we can only store 1024 files per chip (plus a few tiny inline files).
