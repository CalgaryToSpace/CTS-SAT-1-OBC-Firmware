# CTS-SAT-1-OBC-Firmware
Firmware for the CTS-SAT-1 ("FrontierSat") mission. Runs on the STM32-based Onboard Computer.

## Developing with VS Code

1. On Windows only, install the [ST-Link Debugging Driver](https://www.st.com/en/development-tools/stsw-link009.html) for compatibility with OpenOCD.
2. Clone this repo.
3. Open this repo in File Explorer.
4. Right click on the `firmware` folder in this repo, and select "Open with VS Code". **Do not open the entire repo with VS Code like you normally would.**
5. When prompted in the bottom-right corner, select these options:

![VS Code initial opening options](/docs/media/vs-code-startup-instructions.png)

6. Install the recommended VS Code extensions when prompted.
	* If you're not prompted, use Ctrl+Shift+P to navigate to ">Extensions: Show Recommended Extensions".
7. Navigate to the "ST" extension in the extension pane. Select the option to install the build toolchain (appears for first use).
8. In the "ST" extension pane, use the "Build" button to build the repo; or, use Ctrl+Shift+P to open the command palate, search for STM32, and select the Build option.
9. Plug in an STM32 dev kit, and flash the project using the "Flash STM32" button/command palate option.
10. Optional: setup the Codeium extension by creating an account. Codeium is a free GitHub Copilot-like extension which can boost productivity when used effectively.

### Troubleshooting

1. Check that your MicroUSB cable supports data transfer. Many MicroUSB cables are power-only.
2. Disable your antivirus if it starts removing the compiler.
3. Use the Ctrl+Shift+P command palate to run "STM32: Install all the required build ...".
4. 

### Updating the IOC File/Regenerating Boilerplate

The IOC file must be updated using CubeMX instead of STM32CubeIDE now. Feel free to test IOC
file changes, but do not commit IOC file changes nor auto-generated code changes. Instead, once 
you've found the ideal configuration, request the changes to a Team Lead (who will make the change 
directly in the `main` branch).
Then, rebase off of the `main` branch.

## Debugging Setup
* Connect to the STM32 debug serial port at baud=115200.
* Use Breakpoints in VS Code to pause execution and explore the stack. Breakpoint are obeyed when you use the "Debug STM32" button to flash the chip.

The "SerialTest" serial terminal works well for sending commands: https://github.com/wh201906/SerialTest/releases/, and is recommended.

### Python Serial Terminal

The Python serial terminal is useful for receiving only.

```bash
python3 -m pip install pyserial
python3 -m serial.tools.miniterm - 115200
```

## Random Notes
* Lower NVIC priority number means higher priority.
* For UART, FIFO Mode must be enabled.
* When starting a new FreeRTOS project, you must manually increase the total heap size. Hard 
faults may mean a thread is running out of stack memory.
