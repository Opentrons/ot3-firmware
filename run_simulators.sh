trap "exit" INT TERM ERR
trap "kill 0" EXIT

./build-host/head/simulator/head-simulator &
./build-host/gantry/simulator/gantry-x-simulator &
./build-host/gantry/simulator/gantry-y-simulator &
./build-host/pipettes/simulator/pipettes-simulator &
./build-host/gripper/simulator/gripper-simulator &
./build-host/bootloader/simulator/bootloader-simulator &

wait 

