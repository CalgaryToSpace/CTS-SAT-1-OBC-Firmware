# Timestamp Format Rationale

Ensure you've read the [/docs/40_Logging.md](/docs/40_Logging.md) document, which explains the logging timestamp format, before reading this one.

The following example, copied from [a discussion when designing the logging system](https://github.com/CalgaryToSpace/CTS-SAT-1-OBC-Firmware/pull/106#discussion_r1685270996), provides an example of why the sync+source+offset timestamp format is useful.

## Example

### Notation
* Timestamps will be given in the format returned by `TIM_get_timestamp_string()`, but in seconds, and at an arbitrary date.
* Events will be recorded below in chronological order.
* Assume the only sync source is telecommands, right now. 

### Events
1. Satellite powers on at time 1712345000, but assumes it's 1970. 
2. Over the next 300 seconds, a few logs are generated.
```
0000000000_N_0001 [INFO] Satellite Boots
0000000000_N_0010 [INFO] Satellite mounts LittleFS.
0000000000_N_0290 [INFO] Satellite sends radio beacon.
```
3. A telecommand timesync happens, and sets the time to `1712345300`.
4. The satellite logs the time sync:
```
1712345300_T_0001 [INFO] Satellite time synced via telecommand to 1712345300 sec since 1970.
```
5. 500 real seconds pass. The satellite clock ticks fast, and thinks 510 secs pass (this would normally be a huge anomaly, but pretend it's a small discrepancy).
```
1712345300_T_0511 [INFO] A cool action happened and a log message was made
```
6. A second later, a telecommand syncs the time.
```
1712345801_T_0001 [INFO] Satellite time synced via telecommand to 1712345801 sec since 1970
```
7. Observe: Time from Log 5 is 1712345811, and time from log 6 is 1712345802 (when summing the sync+offset). It would seem as though time has "gone backwards", if not for the 2-part logging. However, we can clearly see that the situation is actually just that the clock was running about 10 seconds fast over the 500 secs between syncs, by sorting first by the sync time and then by the offset in the ground station log viewer software (or perhaps Excel haha). This is the non-problematic example of clock drift, and should not generate a warning, as we've designed the logging architecture to handle it.
8. At real time 1712346000, a new operator comes on shift. This operator is really excited, but has fat fingers, and sends a sync command to 1719346000. This is a big jump forward, but it happens anyway. A warning about a large time shift is logged, but the operator misses it. 
```
1719346000_T_0001 [INFO] Satellite time synced via telecommand to 1719346000 sec since 1970
1719346000_T_0001 [WARNING] Satellite observed a large time shift during the resync (this warning is not yet implement, but should be)
```
9. The satellite logs some telemetry/routine actions over the next 200 seconds.
```
1719346000_T_0003 [INFO] Routine action 1
1719346000_T_0020 [INFO] Routine action 2
1719346000_T_0200 [INFO] Routine action 2
```
10. At real time 1712346200, a periodic resync is performed, this time more carefully. 
```
1712346200_T_0001 [INFO] Satellite time synced via telecommand to 1712346200 sec since 1970
1712346200_T_0001 [WARNING] Satellite observed a large time shift during the resync (this warning is not yet implement, but should be)
1712346200_T_0001 [WARNING] (my warning) Setting current time to before the last sync.
```
11. Another 200 seconds pass, and some routine non-telecommand actions on the satellite are logged.
```
1712346200_T_0010 [INFO] Routine action 10
1712346200_T_0020 [INFO] Routine action 11
1712346200_T_0030 [INFO] Routine action 12
```
12. Three days pass. Another operator wants to look at the logs from the past three days, and downlinks them. Due to the lossy nature of the communication link, and automatic repeat requests, the order of the logs is mixed up. Naturally, the logs are re-sorted on the ground using their timestamps.  Sorted, the logs look like:

```
0000000000_N_0001 [INFO] Satellite Boots
0000000000_N_0010 [INFO] Satellite mounts LittleFS.
0000000000_N_0290 [INFO] Satellite sends radio beacon.
1712345300_T_0001 [INFO] Satellite time synced via telecommand to 1712345300 sec since 1970.
1712345300_T_0511 [INFO] A cool action happened and a log message was made
1712345801_T_0001 [INFO] Satellite time synced via telecommand to 1712345801 sec since 1970
1712346200_T_0001 [INFO] Satellite time synced via telecommand to 1712346200 sec since 1970
1712346200_T_0001 [WARNING] (my warning) Setting current time to before the last sync.
1712346200_T_0001 [WARNING] Satellite observed a large time shift during the resync (this warning is not yet implement, but should be)
1712346200_T_0010 [INFO] Routine action 10
1712346200_T_0020 [INFO] Routine action 11
1712346200_T_0030 [INFO] Routine action 12
1719346000_T_0001 [INFO] Satellite time synced via telecommand to 1719346000 sec since 1970
1719346000_T_0001 [WARNING] Satellite observed a large time shift during the resync (this warning is not yet implement, but should be)
1719346000_T_0003 [INFO] Routine action 1
1719346000_T_0020 [INFO] Routine action 2
1719346000_T_0200 [INFO] Routine action 2
```

See that Routine Actions 1,2,3 appear to have happened _after_ 10,11,12. When analyzing the logs, we see the `Setting current time to before the last sync.` message, which gives a hint that the logs are likely out-of-order due to that time sync. 
