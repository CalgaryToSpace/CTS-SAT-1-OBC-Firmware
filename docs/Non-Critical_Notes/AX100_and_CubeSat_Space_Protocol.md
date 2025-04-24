# The AX100 and the CubeSat Space Protocol (`libcsp`)

The GomSpace AX100 is the radio module used on CTS-SAT-1.

## Loading `libcsp` Code from Upstream

The `libcsp` library is loaded from the https://github.com/CalgaryToSpace/libcsp fork, in the `libcsp-cts-sat-1` branch. This branch trails
the `libcsp-1` branch from the upstream repository, and contains tiny build changes to conform to our build system (and warning levels).

In order to maintain a simple build system, we use a script which copies the necessary files from the upstream repository into the `firmware/` directory. This script is maintained at `/misc_tools/copy_in_libcsp.sh`.

## Important Human Communications

### GomSpace Support Ticket, 2024-09-06

> There is not much difference between GomSpace/libcsp and libcsp/libcsp. It is mostly build related changes and a few changes to Python bindings. Note that if you use libcsp/libcsp, you need to use the 1.6 tag or the libcsp-1 branch, as version 2.x is incompatible.

> Note that there is also lib*gs*csp included in the “product interface application”. This is a small wrapper library that includes a few Gomspace specific things, like hooking into Gomspace logging etc. Since you are not using the rest of the Gomspace software stack, I don’t expect there is much need for libgscsp.

## Network Map

* Ground Station CSP Address = d10
* AX100's CSP Address = d5
    * Important for configuring the AX100 on-orbit.
    * Note: AX100's I2C address id d5.
* OBC's CSP Address = d1
    * Note: OBC's I2C Address is d12

## AX100 Configuration

### Routing Table

The following configures uplink and downlink routing.

```
param mem 0
param set csp_rtable "1/5 I2C 12, 10/5 AX100"
```

* To address d1 (with netmask 5 - use all 5 bits as the destination address), use I2C with destination (slave/OBC) I2C address d12.
* To address d10 (with netmask 5 - use all 5 bits as the destination address), use AX100.
    * Ground station address is d10.

* To save, run `param save 0 0`, then `reset`.

* Run `route`, and see the following:

``` 
5/5 LOOP
1/5 I2C 12
10/5 AX100
```

## Troubleshooting

* `ifc` shows packet counts
* `debug frame debug` enables debug logs for all frames in all directions
* `route` shows the currently-active routing table
    * Useful to check that you remembered to reboot, and that the `csp_rtable` parameter is valid.
