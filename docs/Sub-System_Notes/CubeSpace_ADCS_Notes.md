# CubeSpace ADCS Notes


## Cameras

There are two cameras connected to the ADCS.

1. Camera 1: Sun sensor. Requires that the sun is above the local horizon (by configuring the orbit parameters and current time).
    * Senses only very bright lights.
2. Camera 2: Nadir sensor. Pointed at earth. Has a clear view of the boom.
    * Senses only medium-bright lights.


## Quirks and Troubleshooting

1. When doing the SD card index command, if you see that any are in the "Busy Updating" state, then you must wait until none are in the busy updating state before fetching files.
2. When fetching an image from the SD card, if it fails, just try again a few times. Doing `adcs_reset` command can also help resolve that.
