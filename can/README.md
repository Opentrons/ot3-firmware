# CAN LIB

The can communication library.

## Project Structure

### Core

The portable portion of the CAN library. This is built as a shared library and linked with firmware and simulating executbles.

### Firmware

Limited to code that uses the STM32 Hardware Abstraction Layer. These modules are compiled by individual firmware executables. 

### Simulator

Tools to use SocketCAN's VCAN interface for simulation in a Linux system.

### Tests

Unit tests.