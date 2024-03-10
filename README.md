# CTS-SAT-1-OBC-Firmware
Firmware for the CTS-SAT-1 ("FrontierSat") mission. Runs on the STM32-based Onboard Computer.

## Debugging Setup
* Connect to the STM32 debug serial port at baud=1152000.

### Python Serial Terminal

```bash
python3 -m pip install pyserial
python3 -m serial.tools.miniterm - 115200
```

