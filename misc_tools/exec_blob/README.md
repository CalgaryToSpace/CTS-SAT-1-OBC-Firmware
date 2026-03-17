# `exec_blob` Tools

As a somewhat last-resort for missing functionality, and/or for dynamic on-orbit data analysis, it could be helpful to load a machine code file from the filesystem into a buffer in memory, and then execute it as though it's a function.

This functionality is fairly risky (though not too bad, due to safety features like the 2 layers of watchdogs), and decently challenging to use.

* Related to: https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/issues/573
