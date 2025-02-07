# Getting Started!

Welcome! This guide will help you get started with the CTS-SAT-1 OBC Firmware. This firmware runs on the STM32-based Onboard Computer.

Follow through this guide, and follow through the rest of the numbered guides in this `docs/` folder.

## VS Code Setup Procedure

1. On Windows only, install the [ST-Link Debugging Driver](https://www.st.com/en/development-tools/stsw-link009.html) for compatibility with OpenOCD.
    1. Download the file.
    2. Unzip the entire zip file.
    3. Run the `dpinst_amd64.exe` installer.
2. Clone this repo.
3. Open this repo in File Explorer.
4. Right click on the `firmware` folder in this repo, and select "Open with VS Code". **Do not open the entire repo with VS Code like you normally would.**
5. When prompted in the bottom-right corner about "A git repository was found in the parent folders...", click "Yes".
6. Install the recommended VS Code extensions when prompted.
	* If you're not prompted, use Ctrl+Shift+P to navigate to ">Extensions: Show Recommended Extensions".
7. Navigate to the "ST" extension in the extension pane. Select the option to install the build toolchain (appears for first use).
8. In the "ST" extension pane, use the "Build" button to build the repo; or, use Ctrl+Shift+P to open the command palate, search for STM32, and select the Build option.
9. Plug in an STM32 dev kit, and flash the project using the "Flash STM32" button/command palate option.
10. Optional: setup the Codeium extension by creating an account. Codeium is a free GitHub Copilot-like extension which can boost productivity when used effectively.

### Troubleshooting

1. Check that your MicroUSB cable supports data transfer. Many MicroUSB cables are power-only.
2. The dev-board has two connectors on it, only one of which (the CN1 connector) can be used to plug in the MicroUSB cable. Ensure that you are using the correct connector.
3. Disable your antivirus if it starts removing the compiler.
4. Use the Ctrl+Shift+P command palate to run "STM32: Install all the build tools ...".
5. The OBC has two USB connectors which are required for uploading code (the ST-Link) and communicating with a computer (the umbilical). Ensure the correct one(s) are connected to your computer when testing with the OBC.


## Testing and Debugging the Firmware
1. [Install and use the "SerialTest" serial terminal](https://github.com/wh201906/SerialTest/releases/) for sending commands, initially. The [ground support software](https://github.com/CalgaryToSpace/CTS-SAT-1-Ground-Support) is also solid after your first contribution.
    * Connect to the STM32 debug serial port at baud=115200.
2. Flash the board (using the "Flash STM32" button from the setup above).
3. Open the SerialTest terminal, and connect to the STM32 debug serial port at baud=115200.
4. Send a command to the STM32 to test the firmware: `CTS1+hello_world()!`.
5. Verify that you get a nice "Hello World!" response back.

### Techniques for Debugging
* Use `LOG_message()` (or `DEBUG_UART_print_str()`, for temporary logs) to print out intermediate values in the code.
* Use Breakpoints in VS Code to pause execution and explore the stack. Breakpoint are obeyed when you use the "Debug STM32" button to flash the chip.
    * ⚠️ Beware that breakpoints with Embedded Systems can be finicky/cause unpredictable behavior, as they can interfere with interrupts.
