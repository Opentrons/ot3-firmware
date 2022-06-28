# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Gripper")
add_STM32G4_freertos("Gripper")

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(GRIPPER_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_grip_motor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_z_motor.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/utils.c
        ${CAN_FW_DIR}/hal_can.c
        )

set(GRIPPER_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_encoder_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_timer_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

add_executable(gripper
        ${GRIPPER_FW_LINTABLE_SRCS}
        ${GRIPPER_FW_NON_LINTABLE_SRCS})

target_gripper_core(gripper)

target_ot_motor_control(gripper)

target_i2c_firmware(gripper)

target_link_libraries(gripper
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Gripper STM32G4xx_FreeRTOS_Gripper
        can-core)

set_target_properties(gripper
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

target_include_directories(gripper
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_compile_options(gripper
        PUBLIC
        -Wall
        -Werror
        -Wextra
        -Wno-missing-field-initializers
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_include_directories(STM32G4xx_Drivers_Gripper
        PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS_Gripper
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Gripper
        PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS_Gripper
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Gripper
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")

find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")
# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set(GDBINIT_FILE "${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")
set_target_properties(gripper
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${GDBINIT_FILE}")


find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
add_custom_command(OUTPUT gripper.hex
        COMMAND ${CROSS_OBJCOPY} ARGS gripper "-Oihex" gripper.hex
        DEPENDS gripper
        VERBATIM)
add_custom_target(gripper-hex ALL
        DEPENDS gripper.hex)

add_custom_command(OUTPUT gripper.bin
        COMMAND ${CROSS_OBJCOPY} ARGS gripper "-Obinary" gripper.bin
        DEPENDS gripper
        VERBATIM)
add_custom_target(gripper-bin ALL
        DEPENDS gripper.bin)


find_package(Clang)

# runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# which is a catch-all static analyzer/linter
# the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# An odd thing about this target is that it requires the existance of a compiledb, which
# is produced when you build, and may change if you change compilation options, so in a way
# it depends on a build. But we also want to be able to run this when there wasn't a successful
# build, so there's no explicit dependency set.
# This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# include directories. So we can use the actually fairly cool transform command to turn them all into
# extra-arg invocations and it'll figure it out.
set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# This helps with clang accepting what GCC accepts around the implementations of the message queue
list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(gripper-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${GRIPPER_FW_LINTABLE_SRCS} ${GRIPPER_CORE_SRC})
list(APPEND LINT_TARGETS "gripper-lint")
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)


# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(gripper-debug
        COMMAND gripper
        USES_TERMINAL
        )

# Targets to create full image hex file containing both bootloader and application
add_custom_command(
        OUTPUT gripper-image.hex
        DEPENDS gripper-hex bootloader-gripper-hex gripper.hex $<TARGET_FILE_DIR:bootloader-gripper>/bootloader-gripper.hex
        COMMAND ${CMAKE_SOURCE_DIR}/hex_combine.py gripper-image.hex $<TARGET_FILE_DIR:bootloader-gripper>/bootloader-gripper.hex gripper.hex
        VERBATIM)
add_custom_target(gripper-image-hex ALL
        DEPENDS gripper-image.hex)

set(DISCO_FILE "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg")

# Targets to flash bootloader and firmware
add_custom_target(gripper-flash
        COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${DISCO_FILE}" "-c" "program ${CMAKE_CURRENT_BINARY_DIR}/gripper-image.hex;reset;exit"
        VERBATIM
        COMMENT "Flashing board"
        DEPENDS gripper-image-hex)
