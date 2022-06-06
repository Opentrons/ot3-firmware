# CAN LIB

The can communication library.

## Project Structure

### core

The portable portion of the CAN library. This is built as a shared library and linked with firmware and simulating executbles.

### firmware

Limited to code that uses the STM32 Hardware Abstraction Layer. These modules are compiled by individual firmware executables. 

### simlib

Tools to use SocketCAN's VCAN interface for simulation in a Linux system and proprietary socket interface.

For more information on interacting with simulation see [this readme](https://github.com/Opentrons/opentrons/blob/edge/hardware/README.md)..

### tests

Unit tests.
