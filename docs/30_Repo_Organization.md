# Repo Organization

Skim through the Repo Overview section, then jump to the bottom for a treasure hunt activity!

## Repo Overview

This repository is somewhat of a "monorepo", containing several related but not directly dependent projects. The main projects/parts are:
1. `firmware/`: The main firmware for the CTS-SAT-1 OBC, which runs on the STM32-based Onboard Computer.
    * This is the most important part, by far.
2. `cts1_ground_support/`: The ground support software for the CTS-SAT-1 mission.
    * This is a Python-based GUI tool for interfacing with the OBC over UART, and is used for testing and debugging.
    * In the future, this will be expanded to support sending radio telecommands to the satellite on-orbit.
3. `docs/`: Hand-written documentation (mostly guides) for the software components of the projects.

## `firmware/` Organization

The `firmware/` folder contains the main firmware for the CTS-SAT-1 OBC. It is organized as follows:
* `Core/Inc/` has the header files (`.h` files) for the firmware.
* `Core/Src/` has the source files (`.c` files) for the firmware.
    * `main.c` is the entry point for the firmware. Do not edit it (much).
    * `rtos_tasks/rtos_tasks.c` contains the main FreeRTOS tasks (each is a `while(1)` loop that takes turns with the other tasks).
* For `Core/Inc/` and `Core/Src/`:
    * Each subsystem has its own folder.
    * Each C file has an associated header file with the same name.

## `cts1_ground_support/` Organization

The `cts1_ground_support/` folder contains the ground support software for the CTS-SAT-1 mission. It is organized as follows:

* `firmware_checks/`: Scripts for GitHub CI which validate C-to-Python code parsing.
* `src/cts1_ground_support/`: The main Python codebase for the ground support software. Importable as a Python module.
* `tests/`: Unit tests for the ground support software.

## Treasure Hunt Activity

1. Find where the unit tests are stored.
2. Find the documentation that explains unit tests.
3. Find where the telecommand list is stored.
4. Find the documentation that explains the telecommand format, with examples.
5. Find the driver to deploy the deployable antenna. Find the associated telecommand.


## Tips for Navigating in VS Code

* Use `Ctrl+P` to open a pane to type in a filename. Use arrow keys and Enter to navigate.
* Use `Ctrl+Shift+F` to search for a string in all files.
