# Kernel

## Introduction

## Building

You will need: GCC cross compiler targeting aarch64. See [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
or consult your package manager for help obtaining the correct compiler.

To build the kernel, simply run `make` in this directory. `make` will produce an ELF binary, `kernel.elf`,
as well as a flat binary, `kernel8.img`, for running on the Raspberry Pi.

## Running

This kernel has so far been tested on an emulator, [QEMU](https://www.qemu.org/), as well as the Raspberry Pi 3B.

To run this kernel on QEMU, run:
    `qemu-system-aarch64 -M raspi3b -serial stdio -kernel kernel8.img`

To run this kernel on the Raspberry Pi, place `kernel8.img` in the boot partition of your Pi's SD card.
On the Raspberry Pi 3B, you may also need to add the following like to `confix.txt`:
    `arm_64bit=1`
