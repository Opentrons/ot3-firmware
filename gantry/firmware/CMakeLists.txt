# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Gantry")
add_STM32G4_freertos("Gantry")

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(GANTRY_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
        ${COMMON_EXECUTABLE_DIR}/spi/spi_comms.cpp
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/utils.c
        ${CAN_FW_DIR}/hal_can.c
        )

set(GANTRY_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware.c
        ${COMMON_EXECUTABLE_DIR}/spi/spi.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        )

add_executable(gantry-x
        ${GANTRY_FW_LINTABLE_SRCS}
        ${GANTRY_FW_NON_LINTABLE_SRCS})


add_executable(gantry-y
        ${GANTRY_FW_LINTABLE_SRCS}
        ${GANTRY_FW_NON_LINTABLE_SRCS})

target_gantry_core_x(gantry-x)
target_gantry_core_y(gantry-y)

target_ot_motor_control(gantry-x)
target_ot_motor_control(gantry-y)

target_link_libraries(gantry-x
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Gantry STM32G4xx_FreeRTOS_Gantry
        can-core)

target_link_libraries(gantry-y
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Gantry STM32G4xx_FreeRTOS_Gantry
        can-core)


set_target_properties(gantry-x
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

set_target_properties(gantry-y
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)


target_include_directories(gantry-x
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_include_directories(gantry-y
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


target_compile_options(gantry-x
        PUBLIC
        -Wall
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_compile_options(gantry-y
        PUBLIC
        -Wall
        -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)


target_include_directories(STM32G4xx_Drivers_Gantry
        PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS_Gantry
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Gantry
        PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS_Gantry
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Gantry
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
set_target_properties(gantry-x
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${GDBINIT_FILE}")

set_target_properties(gantry-y
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${GDBINIT_FILE}")


find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
add_custom_command(OUTPUT gantry-x.hex
        COMMAND ${CROSS_OBJCOPY} ARGS gantry-x "-Oihex" gantry-x.hex
        DEPENDS gantry-x
        VERBATIM)
add_custom_target(gantry-x-hex ALL
        DEPENDS gantry-x.hex)

add_custom_command(OUTPUT gantry-x.bin
        COMMAND ${CROSS_OBJCOPY} ARGS gantry-x "-Obinary" gantry-x.bin
        DEPENDS gantry-x
        VERBATIM)
add_custom_target(gantry-x-bin ALL
        DEPENDS gantry-x.bin)


find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
add_custom_command(OUTPUT gantry-y.hex
        COMMAND ${CROSS_OBJCOPY} ARGS gantry-y "-Oihex" gantry-y.hex
        DEPENDS gantry-y
        VERBATIM)
add_custom_target(gantry-y-hex ALL
        DEPENDS gantry-y.hex)

add_custom_command(OUTPUT gantry-y.bin
        COMMAND ${CROSS_OBJCOPY} ARGS gantry-y "-Obinary" gantry-y.bin
        DEPENDS gantry-y
        VERBATIM)
add_custom_target(gantry-y-bin ALL
        DEPENDS gantry-y.bin)


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
add_custom_target(gantry-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${GANTRY_FW_LINTABLE_SRCS} ${GANTRY_X_CORE_SRC} ${GANTRY_Y_CORE_SRC})
list(APPEND LINT_TARGETS "gantry-lint")
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)


# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(gantry-debug-x
        COMMAND gantry-x
        USES_TERMINAL
        )

add_custom_target(gantry-debug-y
        COMMAND gantry-y
        USES_TERMINAL
        )


target_stm32G4_flash(gantry-x $<TARGET_FILE_DIR:gantry-x>/gantry-x.bin $<TARGET_FILE_DIR:bootloader-gantry-x>/bootloader-gantry-x.bin)
target_stm32G4_flash(gantry-y $<TARGET_FILE_DIR:gantry-y>/gantry-y.bin $<TARGET_FILE_DIR:bootloader-gantry-y>/bootloader-gantry-y.bin)

