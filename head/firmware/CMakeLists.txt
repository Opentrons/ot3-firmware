# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Head")
add_STM32G4_freertos("Head")

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(HEAD_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/step_motor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/can_task.cpp
        ${COMMON_EXECUTABLE_DIR}/spi/spi_comms.cpp
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/hal_can_message_buffer.cpp
        ${CAN_FW_DIR}/utils.cpp
        )

set(HEAD_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tim7.c
        ${COMMON_EXECUTABLE_DIR}/can/can.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/motor_driver/motor.c
        ${CMAKE_CURRENT_SOURCE_DIR}/spi.c 
        )

add_executable(head
        ${HEAD_FW_LINTABLE_SRCS}
        ${HEAD_FW_NON_LINTABLE_SRCS})

target_link_libraries(head
        PUBLIC STM32G491RETx
        STM32G4xx_Drivers_Head STM32G4xx_FreeRTOS_Head
        can-core)

set_target_properties(head
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

target_include_directories(head
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(head
        PUBLIC
        -Wall
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_include_directories(STM32G4xx_Drivers_Head
        PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS_Head
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Head
        PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS_Head
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Head
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")

find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")
# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set_target_properties(head
        PROPERTIES
        CROSSCOMPILING_EMULATOR
        "${ARM_GDB};--command=${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")


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
add_custom_target(head-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${HEAD_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES} --config=)

# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(head-debug
        COMMAND head
        USES_TERMINAL
        )


# Runs openocd to flash the board (without using a debugger)
add_custom_target(head-flash
        COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg" "-c" "program $<TARGET_FILE:head>;reset;exit"
        VERBATIM
        COMMENT "Flashing board"
        DEPENDS head)