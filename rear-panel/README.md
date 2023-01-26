# Rear Panel
The rear panel is a special exception to the rest of the projects in this repo, since the can bus is not connected to the mcu it uses a usb based update. It uses less of the common code because of this. It also has slightly different linking than the rest of the boards to accomidate the bootloader differences
## Connections
The MCU of this board is connected to the SOM board via USB.

The ESTOP button is connected via two inputs, we do this so we can tell if there the estop button is pressed, released or not plugged in. The MCU will read these two inputs and output the correct estop setting to the rest of the system.

The Deck and UI leds are powered and controlled form this board

There is two AUX inputs for extra devices like the Hepa Hood

There is also two spare IO pins that are currently unused for anything but get passed along the data cable to the y board.


## Building

like the rest of the system the this project uses cmake presets
here is a list of targets that are built by the project
rear-panel-rev1
  build the main binary
rear-panel-rev1-hex
  dump the binary to a hex file
rear-panel-rev1-startup
  build the startup program binary
rear-panel-rev1-startup-hex
  dump the startup binary to hex
rear-panel-rev1-integrity
  calculate the checksum of the main binary
  insert it into that the main binary's .integrity-info section
rear-panel-rev1-image-hex
  combines the hex file for the startup and mainbinary into a single hex for flashing
rear-panel-rev1-flash
  flash just the main program's section of memory and leave the startup app alone
rear-panel-rev1-image-flash
  flash both the startup and the main binary
rear-panel-rev1-debug
  debug the main binary
rear-panel-rev1-startup-debug
  debug the startup app
rear-panel-rev1-clear
  clear the write protect settings for the startup app, you need to call this between calls to rear-panel-rev1-image-flash 

additionally it has the project wide targets such as format, lint etc.

## Flashing

for the first time flash call:
cmake --build --preset rear-panel --target rear-panel-rev1-image-flash
if the project has had the startup app previously flashed to it you will need to call
cmake --build --preset rear-panel --target rear-panel-rev1-clear

if you want to just update the program section 
cmake --build --preset rear-panel --target rear-panel-rev1-flash
