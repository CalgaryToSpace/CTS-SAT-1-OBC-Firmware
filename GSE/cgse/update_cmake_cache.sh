#!/bin/bash

# Run this script from the firmware directory to regenerate the cmake commands
# file used for clangd in neovim.

cmake -S . -G "Unix Makefiles" -B cmake

