# The AX100 and the CubeSat Space Protocol (`libcsp`)

The GomSpace AX100 is the radio module used on CTS-SAT-1.

## Loading `libcsp` Code from Upstream

The upstream https://github.com/libcsp/libcsp repository is used for the code here. The code is used at approximately the v1.6 tag (TBC).

In order to maintain a simple build system, we use a script which copies the necessary files from the upstream repository into the `firmware/` directory. This script is maintained at `/misc_tools/copy_in_libcsp.sh`.

## Important Human Communications

### GomSpace Support Ticket, 2024-09-06

> There is not much difference between GomSpace/libcsp and libcsp/libcsp. It is mostly build related changes and a few changes to Python bindings. Note that if you use libcsp/libcsp, you need to use the 1.6 tag or the libcsp-1 branch, as version 2.x is incompatible.

> Note that there is also lib*gs*csp included in the “product interface application”. This is a small wrapper library that includes a few Gomspace specific things, like hooking into Gomspace logging etc. Since you are not using the rest of the Gomspace software stack, I don’t expect there is much need for libgscsp.
