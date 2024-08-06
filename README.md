# ot3_firmware

This repository holds the firmware for the Flex and all of its peripheral systems. **Note** that the cmake directory is
actually a subtree of cmake-utils. Do not make changes to the `cmake-utils` directory in the ot3_firmware repository.
Instead, navigate to the [CMake Utils Repository](https://github.com/Opentrons/cmake-utils).

## Structure of directories

Aside from the common directory, each repository should contain a `firmware`, `include`, `src`, `tests` folder.

1. `include` should hold any header file that crosses subsystem boundaries - public interfaces. This folder should include another subset of directories labeled `firmware`, `src` and `tests` for import cleanness.
2. Source subdirectories for end executables - `pipettes`, `head`, `gantry` - and subsystems - `can`,`motor-control`. Each should have subdirectories for `core`, `tests`, `firmware`, and the like.
3. `python` is a subdirectory for python bindings and utilities, providing python bindings for canbus message definitions. It can be used by external python projects to deal with serializing and unserializing these messages, as well as being a source of truth for generating IDs in c++.

## Revisions

Each application - the executables that we compile to run on a specific microcontroller - is compiled for all the different PCBA revisions that we support. The revisions are defined based on the actual PCBA revisions that get manufactured. The expectation is that we'll always maintain firmware compatible with each revision, and pack executables for each revision of each PCBA into the robot software for in-field updates.

The meaning of a revision is aligned with Opentrons electrical engineering product lifecycle management. A PCBA revision is PN.M, where:

- P is the primary revision and is a letter. The primary revision changes when traces, drills, manufacturing notes, etc on the PCBA designs change - anything that would require different board fabs.
- N is the secondary revision and is a number. The secondary revision changes when a schematic, assembly note, component, etc on the PCBA designs change - anything that has the same fabs, but functionally different PCBAs
- M, which we don't care about, is the tertiary revision, and is a change that does not result in a functional change.

Since we don't care about tertiary revisions, we compile firmware for each primary-secondary revision that gets manufactured, and we expose the primary and secondary revisions to the firmware in the generated revisions.c file, which is accessible by including versions.h.

Application binaries are generated with different names for each revision and can be compiled separately by target - e.g. `cmake --build --preset=firmware-g4 --target gantry-x-b1-hex` generates the gantry-x firmware for revision B1, `cmake --build --preset=firmware-g4 --target pipettes-single-c2-image-hex` generates the single pipette firmware image (with bootloader) for revision c2. All revisions for a single application binary type (e.g. image, application, executable) are wrapped up in a target, so `cmake --build --preset=firmware-g4 --target gripper-images` builds the images hex files for all gripper revisions, `cmake --build --preset=firmware-g4 --target head-applications` builds the application hex files for all head revisions.

This can end up being quite a lot of hex files scattered in various subdirectories, so if you run `cmake --install` cmake will helpfully copy them all to the `dist/` subdirectory for you. You can change where it installs by changing `CMAKE_INSTALL_PREFIX`.

The `--install` command will now helpfully build the manifest required for firmware update. That means that you can do `cmake --build --preset=firmware-g4 --target firmware-applications`, then `cmake --install ./build-cross --component Applications` and then you'll have everything you need in `dist/applications` to do firmware update, so you can do `scp ./dist/applications/* robotip:/usr/lib/firmware/` and get things restarted. This is not currently integrated into cmake because the robot ip is a runtime argument which is hard to get cmake to want to do.

## Working with CMake

This project uses cmake as a build and configuration system. It uses [cmake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html) to ease remembering commands. It requires at least CMake 3.20 (to support build presets) to run.

Using a preset is required for configuration to prevent mistakes that can create directories all over the place.

To list available configuration presets, run `cmake --list-presets`. You can then run `cmake --preset=selected-preset .` to configure, which will create an appropriate binary directory.

Building has presets now too. You can run `cmake --build --list-presets` to show available build presets. They're linked to configuration presets, and you can run them with `cmake --build --preset=selected preset` to build their default target in the appropriate binary directory.

When building, even with a preset, you can set any target you want with `--target`. To prevent mistakenly using a target from the wrong cross configuration, executables all have their own presets, which depend on the correct configuration. When building a target, always try and use the preset that matches that target. For instance, if you're trying to debug a gantry board, run `cmake --build --preset=gantry --target gantry-debug`.

## Setup

To setup this directory to run on an STM32G4 system board (gantry and head), you should run:

1. `cmake --preset=cross .`
2. `cmake --build --preset=gantry --target <TARGET>` or `cmake --build --preset=head --target <TARGET>`
3. To build a specific gantry you can also use the `gantry_x` and `gantry_y` targets

To setup this directory to run tests, you should run:

1. `cmake --preset=host .`
   If you are on OSX, you almost certainly want to force cmake to select gcc as the compiler used for building tests,
   because the version of clang built into osx is weird. We don't really want to always specify the compiler to use in
   tests, so forcing gcc is a separate cmake config preset, and it requires installing gcc 10:

   `brew install gcc@10`
   `cmake --preset=host-gcc10 .`
   The python bindings require specific versions of python - python 3.7 for now. As with the monorepo, the best way
   to handle this without altering your main system is to install and use [pyenv](https://github.com/pyenv/pyenv),
   which will be respected by the cmake infrastructure here. A pyenv configuration file is in the root of the repo.
2. `cmake --build ./build-host --target build-and-test`, which will run all of the tests available in each periphery.
3. or, `cmake --build ./build-host --target <TARGET>-build-and-test`

### Gantry Subsystem

While the `gantry` target builds both, there are separate firmwares (and separate cmake targets) for x and y:

1. `cmake --preset=cross .`
2. `cmake --build --preset=gantry-x` or `cmake --build --preset=gantry-x --target=gantry-debug-x`
3. `cmake --build --preset=gantry-y` or `cmake --build --preset=gantry-y --target gantry-debug-y`

### Cross-compiling vs Host-compiling

As you saw above, two different presets were used to run tests vs running on a physical device. Cross-compiling will
generally be used when the code needs to be ported to other devices (in this case the microcontroller) while
host-compiling is used when you want to run things like tests. For more information check out this
helpful [article](https://landley.net/writing/docs/cross-compiling.html).

## Integrating CANbus definition changes from the Opentrons monorepo

The CANbus node ids, message ids, enums, and definitions are set in
[the Opentrons monorepo](https://github.com/Opentrons/opentrons/tree/edge/hardware/opentrons_hardware/firmware_bindings/constants.py)
and generated to c++ and c. To avoid convoluted automation setups, we manually run the header generation when we
need to make changes. To generate these headers, you can run `cmake --build --preset=<any preset> --target update-headers`.

This requires the Opentrons monorepo to be checked out somewhere and found by CMake. If you check it out as a sibling
to where you check out this repository, it will be found automatically; otherwise, pass 
`-DOPENTRONS_HARDWARE_IMPORT_PATH=/path/to/opentrons_hardware` when you run a cmake configure. For instance, if you have the
monorepo checked out to `/my/favorite/path/opentrons`, you would pass
`-DOPENTRONS_HARDWARE_IMPORT_PATH=/my/favorite/path/opentrons/hardware/opentrons_hardware`, the path containing the
opentrons_hardware source. 

If you don't have constants.py available, everything will still build and run fine, but you won't be able to generate new
header files.

## Run

Connect to an STM32 nucleo board and run either:

1. `make <TARGET>-debug` inside the build-cross folder
2. or, `cmake --build ./build-cross --target <TARGET>-debug` from the top-level folder.

### Debug

If you run into trouble starting a gcc connection to your nucleo board, you should (while connected to the board):

Start openocd:

1. Navigate to `stm32-tools/openocd/Darwin`
2. `./bin  scripts/board/st_nucleo_f3.cfg` (or `scripts/board/st_nucleo_g4.cfg` for gantry/head)

Start gcc:

1. `./stm32-tools/gcc-arm-embedded/Darwin/bin/arm-none-eabi-gdb-py <TARGET>`
2. `target extended-remote :3333`

And then run all the lines before the error to see what is causing the problem.

## Common

The common directory will hold code that should be shared amongst all peripheral systems.

## Pipettes

This is the top-level directory for running the pipette peripheral system. Eventually, we will most likely have separate
directories for different applications on the pipettes.

## Simulate

Each peripheral system can be run in a simulating mode. Because we use the posix FreeRTOS port everything above the HAL is portable this mode is perfect for testing CAN communication and other business logic.

### Building

`cmake --build --preset=simulators` will build all the applications.

### CAN Communication

Two modes of CAN communication are supported:

- socketcan (linux only)
- opentrons_sock (mac + linux)

To use socket_can, define the environment variable `USE_SOCKETCAN` during the build.

For more information on interacting with simulation see [this readme](https://github.com/Opentrons/opentrons/blob/edge/hardware/README.md).

### Running

Each simulator is an executable binary using the pattern `build-host/{PROJ}/simulator/{PROJ}-simulator`

The script `run_simulators.sh` will run all the simulators.

### Environment Variables

The simulators can be customized using environment variables.

#### Socket CAN

`CAN_CHANNEL` - is the SocketCAN channel to use.

#### Opentrons Socket

`CAN_SERVER_HOST` - Host name of opentrons can socket server

`CAN_PORT` - Port of opentrons can socket server

## FW Update

Firmware update can be performed over the CAN bus.

### Prerequisites

- A bootloader must be flashed onto a device in order to perform FW update over CAN. A bootloader can be flashed using one of the `bootloader-{subsystem}-flash` targets.
- A CAN analyzer or other CAN connection to device.

### Instructions

1. Create a `hex` file for the subsystem to be updated. There are `{subsystem}-hex` targets for each subsystem.
2. Use [update firmware](https://github.com/Opentrons/opentrons/tree/edge/hardware#opentrons_update_fw) script.

## code coverage

This repository is configured to generate code coverage from the tests.

To enable code coverage, configure the host toolchain for tests with the options `-DENABLE_COVERAGE=On -DCMAKE_BUILD_TYPE=Debug`. _In order for code coverage to be accurate, you must run the build-and-test target for all targets_. After running every build-and-test target, the target `lcov` will generate an HTMl report of the code coverage from the tests.

In general, do not enable code coverage unless you need it. Test execution will be far slower.

## Tags, Flex Build Triggers, and Releases

- pushes to `main` trigger Flex builds over in <https://github.com/Opentrons/oe-core>
- `v*` and `*@*` tags are only used by the Robot Stack release process
- pushes of `v*` and `*@*` do **NOT** trigger Flex builds
- firmware release bundles happen on all pushes and `v*` tag
- PRs with `flex-build` contained in the branch name kick off Flex builds.
  - this is the developer workflow for getting the builds you need over in <https://github.com/Opentrons/oe-core>
