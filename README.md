# How to build

## If  you don't have it already get an ARM GCC toolchain

https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads

## Get mbed

git clone git@github.com:ARMmbed/mbed-os.git

## Create mbed_settings.py

Just add the location of the bin directory for your ARM GCC toolchain: 

Example
from os.path import join, abspath, dirname
GCC_ARM_PATH = "/home/linux/Downloads/gcc-arm-none-eabi-7-2017-q4-major/bin"

## Build project

mbed compile -t GCC_ARM -m <TARGET_BOARD_NAME>

##Deploy

Just copy the generated bonary file to your target board storage folder, for example:
sudo cp ./<TARGET_BOARD_NAME>/GCC_ARM/<PROJECT_NAME>.bin /media/linux/<TARGET_BOARD_STORAGE>/
