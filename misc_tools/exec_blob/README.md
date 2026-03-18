# `exec_blob` Tools

As a somewhat last-resort for missing functionality, and/or for dynamic on-orbit data analysis, it could be helpful to load a machine code file from the filesystem into a buffer in memory, and then execute it as though it's a function.

This functionality is fairly risky (though not too bad, due to safety features like the 2 layers of watchdogs), and decently challenging to use.

* Related to: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/573

## Status

Basic demo blobs work reliably and safely, so this feature will be included in the firmware in case it is needed.

## Guide

1. Duplicate `demo_blob_2/` to start your own blob/script/payload.
2. Modify `blob_main.c` to implement whatever you wish.
3. Use the `arm-none-eabi-nm -n CTS-SAT-1_FW.elf | grep function_or_variable_name` command to get addresses of functions in the main firmware.
    * Be sure to add one to the address of functions (make the address odd).
4. Run `make clean && make`
5. Convert the `blob.bin` output file to hex and upload it using `fs_write_file_hex(...)`, or use the bulk file uplink procedure.
6. Verify the hash with the SHA256 telecommand.
7. Run the `exec_blob_from_fs` telecommand to run the new blob.
    * The blob is triggered with the remaining/unused argument: `exec_blob_from_fs(blob_filename.bin,0,this_argument_is_passed_as_arg_str_to_blob)`


## Other Notes

* When using `where_to_load_arg_1=1` or `2`, MPI data collection may not work well while using this feature (though will return to normal after the completion of `exec_blob_from_fs`), as we use the giant MPI buffer to store the blob/program.
    * Using `where_to_load_arg_1=0` allows MPI operations to work as normal.
* This feature is "secure" because uplinks are validated/authenticated by HMAC in the AX100 transceiver.
