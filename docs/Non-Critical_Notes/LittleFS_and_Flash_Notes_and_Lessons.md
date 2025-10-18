# LittleFS and Flash Memory - Notes and Lessons

The LittleFS filesystem is used to provide a layer on top of the raw flash memory module (block device) which allows for files/folders, and other standard filesystem features (reading, appending, etc.).

## Flash Memory Module

* Chip: MT29F1G 1Gib (128MiB)
    * NAND FLASH
    * Datasheet: https://www.farnell.com/datasheets/3151163.pdf
* SPI DMA is faster.
* The MT29F1G chip requires that the CS line be driven to inactive between writes, otherwise the system goes inactive.

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

