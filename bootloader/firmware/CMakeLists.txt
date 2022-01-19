# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Bootloader")

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

# Add source files that should be checked by clang-tidy here
set(BOOTLOADER_FW_LINTABLE_SRCS
        )

set(BOOTLOADER_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c)

add_executable(bootloader
        ${BOOTLOADER_FW_LINTABLE_SRCS}
        ${BOOTLOADER_FW_NON_LINTABLE_SRCS})

target_link_libraries(bootloader
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Bootloader)

set_target_properties(bootloader
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

target_include_directories(bootloader
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(bootloader
        PUBLIC
        -Wall
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_include_directories(STM32G4xx_Drivers_Bootloader
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Bootloader
        PUBLIC STM32G491xx)

find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")
# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set_target_properties(bootloader
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")

find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
add_custom_command(OUTPUT bootloader.hex
        COMMAND ${CROSS_OBJCOPY} ARGS bootloader "-Oihex" bootloader.hex
        DEPENDS bootloader
        VERBATIM)
add_custom_target(bootloader-hex ALL
        DEPENDS bootloader.hex)

add_custom_command(OUTPUT bootloader.bin
        COMMAND ${CROSS_OBJCOPY} ARGS bootloader "-Obinary" bootloader.bin
        DEPENDS bootloader
        VERBATIM)
add_custom_target(bootloader-bin ALL
        DEPENDS bootloader.bin)


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
add_custom_target(bootloader-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${BOOTLOADER_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
list(APPEND LINT_TARGETS bootloader-lint)
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(bootloader-debug
        COMMAND bootloader
        USES_TERMINAL
        )


# Runs openocd to flash the board (without using a debugger)
add_custom_target(bootloader-flash
        COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg" "-c" "program $<TARGET_FILE:bootloader>;reset;exit"
        VERBATIM
        COMMENT "Flashing board"
        DEPENDS bootloader)
