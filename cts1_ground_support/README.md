# CTS-SAT-1 Ground Support Software (`cts1_ground_support`)

This ground support software is used to test the satellite by sending the satellite telecommands,
primarily over the debug UART interface. It is a command-aware UART terminal.

Over time, the goal is to develop it into the ground station control software.

## Goals

1. Increase ease of testing during development.
2. Enable automated testing via telecommands to rapidly test for regressions during development.

## Features

* Python-based.
* Serial communication with the OBC (Onboard Computer), which runs the firmware in this repo.
* Automatically parses the C code and loads the list of telecommands, along with certain associated metadata (including documentation), from this git repo.
* Executes automated test procedures by executing telecommands, and assessing the response(s).

## Getting Started

1. Install Python 3.11 from the Microsoft Store.
2. Clone this repo, and open a terminal in this repo's root.
3. Run `python3.11 -m venv venv/` to create a Python virtual environment named `venv/` in the current directory.
4. Run `./venv/Scripts/activate` (on Windows), or `source ./venv/bin/activate` (on real operating systems).
5. Run `pip install -e cts1_ground_support[dev]` to install this Python Project in your virtual environment.
6. Run `cts1_ground_support --help` to ensure it worked.
7. Run `cts1_ground_support` to start the ground support terminal.
8. Visit [http://127.0.0.1:8050/](http://127.0.0.1:8050/) in a web browser to view the web interface and send commands to your dev kit.
