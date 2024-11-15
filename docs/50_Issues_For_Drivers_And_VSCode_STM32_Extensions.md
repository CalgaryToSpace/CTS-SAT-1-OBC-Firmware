### VS Code Setup Procedure (ARM64 Computers)

## Driver installation issues. (ARM64)
If installing the drivers doesn't work follow these instructions:
1. [ARM64 Forum](https://community.st.com/t5/stm32-mcus-boards-and-hardware/stlink-stcubeprogrammer-support-on-windows-arm64/td-p/224127) From the following ARM64 forum scoll down to the accepted solution. There should be a file called stsw-link009_v3_ARM64_UNSIGNED.zip from user CPop.1. Install and unzip file.
2. Click on Windows icon > Shut down or sign out > hold Shift and press Restart. 
3. The computer should restart and open with options. In this order click Troubleshoot > Advanced options > Startup Settings > Restart. The computer shouldrestart with multiple options. input the option for `Disable driver signature enforcement`.
4. Computer should boot like normal. Run the `dpinst_amd64.exe` and follow their instrucations. 
5. Installer should now be installed.

## VS Code setup
If STM32 for VSCode does not install build tools please follow these instructions:
1. Clone this repo.
2. Open this repo in File Explorer.
3. Right click on the `firmware` folder in this repo, and select "Open with VS Code". **Do not open the entire repo with VS Code like you normally would.**
4. When prompted in the bottom-right corner about "A git repository was found in the parent folders...", click "Yes".
5. Install the recommended VS Code extensions when prompted.
	* If you're not prompted, use Ctrl+Shift+P to navigate to ">Extensions: Show Recommended Extensions".
6. Follow instructions from [Chocolatey](https://chocolatey.org/install).
7. Open a powershell terminal and type `choco install gcc-arm-embedded` when prompted just respond wit `y`. Repeat steps for `choco install make`, and `choco install openocd`
8. In the STM32 for VScode page there should be a option to check for build tools and click it. 
9. There should be 3 slots for directorys. If directory says `False` replace with `C:\ProgramData\chocolatey\bin`. Then in STM32 extension click `check build tools`.
9. In the "ST" extension pane, use the "Build" button to build the repo; or, use Ctrl+Shift+P to open the command palate, search for STM32, and select the Build option.
10. Plug in an STM32 dev kit, and flash the project using the "Flash STM32" button/command palate option.