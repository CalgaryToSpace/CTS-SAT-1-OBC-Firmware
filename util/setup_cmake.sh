#!/bin/bash

# run from top level directory
cp util/clangd .clangd 
cp util/update_cmake_cache.sh update_cmake_cache.sh
cp util/STM32Make.make STM32Make.make
cp util/CMakeLists.txt.root CMakeLists.txt
cp util/CMakeLists.txt.firmware firmware/CMakeLists.txt
mkdir -p GSE
cp util/CMakeLists.txt.GSE GSE/CMakeLists.txt


