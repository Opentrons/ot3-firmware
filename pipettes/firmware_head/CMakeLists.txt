# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Pipettes_Head")
add_STM32G4_freertos("Pipettes_Head")

find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")

find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(DISCO_FILE "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg")



set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

set(IMAGES)
set(EXES)

target_include_directories(STM32G4xx_Drivers_Pipettes_Head
        PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS_Pipettes_Head
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Pipettes_Head
        PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS_Pipettes_Head
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Pipettes_Head
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")


find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)

find_package(Clang)


# Add source files that should be checked by clang-tidy here
set(HEAD_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        ${CAN_FW_DIR}/hal_can.c
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/pipette_motor_hardware.cpp
        )

set(HEAD_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/mount_detect_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_hardware.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

add_executable(pipettes-head
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_rev1.c
        ${HEAD_FW_LINTABLE_SRCS}
        ${HEAD_FW_NON_LINTABLE_SRCS})

target_i2c_firmware(pipettes-head)
target_ot_motor_control(pipettes-head)

target_link_libraries(pipettes-head
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Pipettes STM32G4xx_FreeRTOS_Pipettes_Head
        can-core common-core
        )

set_target_properties(pipettes-head
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

target_include_directories(
        pipettes-head
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(pipettes-head
        PUBLIC
        -Wall
        -Wextra
        -Wno-missing-field-initializers
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_pipettes_core_common(pipettes-head "rev1")


# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set_target_properties(pipettes-head
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")

find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
add_custom_command(OUTPUT pipettes-head.hex
        COMMAND ${CROSS_OBJCOPY} ARGS pipettes-head "-Oihex" pipettes-head.hex
        DEPENDS pipettes-head
        VERBATIM)
add_custom_target(pipettes-head-hex ALL
        DEPENDS pipettes-head.hex)

add_custom_command(OUTPUT pipettes-head.bin
        COMMAND ${CROSS_OBJCOPY} ARGS pipettes-head "-Obinary" pipettes-head.bin
        DEPENDS pipettes-head
        VERBATIM)
add_custom_target(pipettes-head-bin ALL
        DEPENDS pipettes-head.bin)

# # runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# # which is a catch-all static analyzer/linter
# # the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# # An odd thing about this target is that it requires the existance of a compiledb, which
# # is produced when you build, and may change if you change compilation options, so in a way
# # it depends on a build. But we also want to be able to run this when there wasn't a successful
# # build, so there's no explicit dependency set.
# # This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# # own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# # include directories. So we can use the actually fairly cool transform command to turn them all into
# # extra-arg invocations and it'll figure it out.
set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# This helps with clang accepting what GCC accepts around the implementations of the message queue
list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(pipettes-head-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} "--extra-arg=-DOPENTRONS_CLANG_TIDY_WORKAROUND_44178" ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${PIPETTE_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
list(APPEND LINT_TARGETS pipettes-head-lint)
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(pipettes-head-debug
        COMMAND pipettes-head
        USES_TERMINAL
        )


# Targets to create full image hex file containing both bootloader and application
add_custom_command(
        OUTPUT pipettes-head-image.hex
        DEPENDS pipettes-head-hex bootloader-pipettes-rev1-hex $<TARGET_FILE_DIR:bootloader-pipettes-rev1>/bootloader-pipettes-rev1.hex pipettes-head.hex
        COMMAND ${CMAKE_SOURCE_DIR}/hex_combine.py pipettes-head-image.hex $<TARGET_FILE_DIR:bootloader-pipettes-rev1>/bootloader-pipettes-rev1.hex pipettes-head.hex
        VERBATIM)
add_custom_target(pipettes-head-image-hex ALL
        DEPENDS pipettes-head-image.hex)
list(APPEND IMAGES pipettes-head-image-hex)

set(DISCO_FILE "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg")

# Targets to flash bootloader and firmware
add_custom_target(pipettes-head-flash
        COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${DISCO_FILE}" "-c" "program ${CMAKE_CURRENT_BINARY_DIR}/pipettes-head-image.hex;reset;exit"
        VERBATIM
        COMMENT "Flashing board"
        DEPENDS pipettes-head-image-hex)

add_dependencies(pipettes-rev1 pipettes-head)

add_custom_target(pipettes-rev1-pipettes-head-images-hex DEPENDS ${IMAGES})

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)

