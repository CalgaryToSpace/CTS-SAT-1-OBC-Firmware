# CTS-SAT-1-OBC-Firmware
Firmware for the CTS-SAT-1 ("FrontierSat") mission. Runs on the STM32-based Onboard Computer.

## Opening with VS Code

1. Clone this repo
2. Open this repo in File Explorer.
3. Right click on the `firmware` folder in this repo, and select "Open with VS Code". Do not open the entire repo with VS Code like you normally would.
4. When prompted in the bottom-right corner, select these options:

![VS Code initial opening options](/docs/media/vs-code-startup-instructions.png)

5. Install the recommended extensions when prompted.
6. Navigate to the "ST" extension in the extension pane, and use the "Build" button to build the repo.
7. Plug in an STM32 dev kit, and flash the project using the "Flash STM32" button.
8. Optional: setup the Codeium extension by creating an account. Codeium is a free GitHub Copilot-like extension which can boost productivity when used effectively.

## Debugging Setup
* Connect to the STM32 debug serial port at baud=1152000.
* Use Breakpoints in VS Code to pause execution and explore the stack. Breakpoint are obeyed when you use the "Flash STM32" button.

### Python Serial Terminal

```bash
python3 -m pip install pyserial
python3 -m serial.tools.miniterm - 115200
```

