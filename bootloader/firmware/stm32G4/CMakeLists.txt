# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)

add_STM32G4_driver("Bootloader")


set(BOOTLOADER_G4FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c)

macro(target_stm32G4 TARGET)
    ## todo: The bootloader project should have its own linker files. It cannot share the other subproject's
    target_link_libraries(${TARGET}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_Bootloader)

    set_target_properties(${TARGET}
            PROPERTIES CXX_STANDARD 20
            CXX_STANDARD_REQUIRED TRUE
            C_STANDARD 11
            C_STANDARD_REQUIRED TRUE)

    target_include_directories(${TARGET}
            PUBLIC ${CMAKE_SOURCE_DIR}/include)

    target_compile_options(${TARGET}
            PUBLIC
            -Wall
            -Werror
            $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
            $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
            $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
            $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
            $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
            $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

    find_program(ARM_GDB
            arm-none-eabi-gdb-py
            PATHS "${CrossGCC_BINDIR}"
            NO_DEFAULT_PATH
            REQUIRED)
    message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")
    # Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
    # this dir
    set_target_properties(${TARGET}
            PROPERTIES
            CROSSCOMPILING_EMULATOR
            "${ARM_GDB};--command=${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")

    find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
            PATHS "${CrossGCC_BINDIR}"
            NO_DEFAULT_PATH
            REQUIRED)
    add_custom_command(OUTPUT ${TARGET}.hex
            COMMAND ${CROSS_OBJCOPY} ARGS ${TARGET} "-Oihex" ${TARGET}.hex
            DEPENDS ${TARGET}
            VERBATIM)
    add_custom_target(${TARGET}-hex ALL
            DEPENDS ${TARGET}.hex)

    add_custom_command(OUTPUT ${TARGET}.bin
            COMMAND ${CROSS_OBJCOPY} ARGS ${TARGET} "-Obinary" ${TARGET}.bin
            DEPENDS ${TARGET}
            VERBATIM)
    add_custom_target(${TARGET}-bin ALL
            DEPENDS ${TARGET}.bin)

    # Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
    # arguable misuse of the concept) to the appropriate cross-gdb with
    # remote-target. You should make sure st-util is running; that's not
    # done here because it won't be multi-os compatible, and also it
    # should be running the entire time and that's tough to accomplish
    # in a custom command
    add_custom_target(${TARGET}-debug
            COMMAND ${TARGET}
            USES_TERMINAL
            )

    # Runs openocd to flash the board (without using a debugger)
    add_custom_target(${TARGET}-flash
            COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg" "-c" "program $<TARGET_FILE:${TARGET}>;reset;exit"
            VERBATIM
            COMMENT "Flashing board"
            DEPENDS ${TARGET})
endmacro()


add_executable(bootloader-head
        ${BOOTLOADER_FW_LINTABLE_SRCS}
        ${BOOTLOADER_FW_NON_LINTABLE_SRCS}
        ${BOOTLOADER_G4FW_NON_LINTABLE_SRCS})

target_stm32G4(bootloader-head)

target_include_directories(STM32G4xx_Drivers_Bootloader
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Bootloader
        PUBLIC STM32G491xx)
