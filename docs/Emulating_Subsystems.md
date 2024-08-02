# Emulating Subsystems

Some subsystems (e.g, EPS, GPS, MPI) are expensive devices, and/or can only be used in certain at-school environments. The facilitate development and testing, we can emulate these subsystems using software running on a computer and/or Arduino.

By using a USB-to-UART converter plugged into the STM32 dev kit on the UART port that the subsystem is expected, and then by running a Python script (which mimics the messaging of the subsystem), we can test the firmware as though it's connected to the actual subsystem.

## Python Emulation Scripts

Emulation scripts exist for the following subsystems, in the following modes:

* GPS: Dump GPS data to the STM32 once every second.
* MPI: Respond to MPI commands from the STM32.
* MPI: Dump MPI science data to the STM32 very rapidly.
    * TODO: implement/check on this one ^

## Python Serial Terminal

The Python serial terminal is useful for receiving only.

For example, if you are testing a system which sends a command to the GPS, for example, you can use this to view the commands sent from the STM32 to the GPS, assuming the USB-to-UART converter is connected to the GPS UART lines on the STM32.

```bash
# Install the Python serial terminal. Only need to run one time.
python3 -m pip install pyserial

# Run the terminal, in basic mode, with 115200 baud rate.
python3 -m serial.tools.miniterm - 115200

# Run the terminal, showing incoming data in hex.
python3 -m serial.tools.miniterm - 115200 --encoding hexlify
```
