# ISISpace EPS Notes

## EPS Lessons Learned
1. The Software ICD suggests that "waiting 20ms after transmitting" is enough to ensure you'll get a good response, but that polling is another option. This is a lie; polling appears to be the only option, as the reception is unreliable no matter how long you wait.
2. The no_operation command doesn't appear to update the result register, and thus the RC (CC+1) byte in the response still shows the previous command's execution when using the no_operation command.
3. The watchdog timer must be serviced as part of the "MVP" implementation.

### EPS via I2C

The EPS is connected by UART, as I2C was not working reliably. The following are notes are about the now-unused I2C implementation.

1. For I2C on the STM32, you must left-shift the address by 1, compared to what's in the Software ICD.
2. From the start, write out the TX and RX bytestreams to a debug UART when working with I2C.
