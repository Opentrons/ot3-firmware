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


## Simulator

This is an application that uses SocketCan vcan (virtual) driver and runs only on linux. It will loopback commands that it receives.

### Building the simulator (Linux Only!)

- Follow the setup guide in [README](../README.md)
- run `cmake --build ./build-host --target can-simulator`

### Start VCAN

- add link `sudo ip link add dev vcan0 type vcan fd on`
- start it up `sudo ip link set up vcan0`

### Configuring

By default, the simulator will use `vcan0`. To override this, set the `CAN_CHANNEL` environment variable.
