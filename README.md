# ot3_firmware

This repository holds the firmware for the OT-3 and all of its peripheral systems. **Note** that the cmake directory is actually a subtree of cmake-utils. Do not make changes to the `cmake-utils` directory in the ot3_firmware repository. Instead, navigate to the [CMake Utils Repository](https://github.com/Opentrons/cmake-utils). 

## Structure of directories
Aside from the common directory, each repository should contain a `firmware`, `include`, `src`, `tests` folder.
1. `firmware` should hold code that will be controlling the peripheral system.
2. `include` should hold the majority of the header files you will need. This folder should include another subset of directories labeled `firmware`, `src` and `tests` for import cleaness.
3. `src` should include any libraries, 3D party or otherwise, that are required for `firmware` to control the peripheral system.
4. `tests` should include tests for the firmware folder.

## Setup
To setup this directory to run on a nucleo board, you should run:
1. `cmake --preset=cross`
2. `cmake --build ./stm32-cross`

To setup this directory to run tests, you should run:
1. `cmake --preset=stm32-host .`
2. `cmake --build ./build-stm32-host --target build-and-test`, which will run all of the tests available in each periphery.
3. or, `cmake --build ./build-stm32-host --target <TARGET>-build-and-test`

### Cross-compiling vs Host-compiling
As you saw above, two different presets were used to run tests vs running on a physical device. Cross-compiling will generally be used when the code needs to be ported to other devices (in this case the microcontroller) while host-compiling is used when you want to run things like tests. For more information check out this helpful [article](https://landley.net/writing/docs/cross-compiling.html).

## Run
Connect to an STM32 nucleo board and run either:
1. `make <TARGET>-debug` inside the build-cross folder
2. or, `cmake --build ./build-stm32-cross --target <TARGET>-debug` from the top-level folder.

### Debug
If you run into trouble starting a gcc connection to your nucleo board, you should (while connected to the board):

Start openocd:

`./bin  Scripts/Darwin/board/st_nucleo_f3.cfg`

Start gcc:
1. `./stm32-tools/gcc-arm-embedded/Darwin/bin/arm-none-eabi-gdb-py <TARGET>`
2. `target extended-remote :3333`

And then run all the lines before the error to see what is causing the problem.

## Common
The common directory will hold code that should be shared amongst all peripherial systems.

## Pipettes
This is the top-level directory for running the pipette perihperal system. Eventually, we will most likely have separate directories for different applications on the pipettes.
