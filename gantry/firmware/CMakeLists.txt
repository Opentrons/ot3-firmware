# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Gantry")
add_STM32G4_freertos("Gantry")

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


set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

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

# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set(GDBINIT_FILE "${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")

set(DISCO_FILE "${COMMON_EXECUTABLE_DIR}/STM32G491RETx/stm32g4discovery.cfg")

# List of revisions of the gantry boards that we support
set(REVISIONS proto rev1)
# The default revision that you'll get if you use targets not qualified
# by revision (i.e. if you do cmake --build --preset=gantry --target gantry-x
# rather than cmake --build --preset=gantry --target gantry-x-proto)
set(DEFAULT_REVISION proto)

set(AXES x y)


# Add source files that should be checked by clang-tidy here
set(GANTRY_FW_LINTABLE_SRCS_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
    ${CAN_FW_DIR}/hal_can_bus.cpp
    ${CAN_FW_DIR}/utils.c
    ${CAN_FW_DIR}/hal_can.c
)

set(GANTRY_FW_LINTABLE_SRCS_rev1
    ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_rev1.cpp
)
set(GANTRY_FW_LINTABLE_SRCS_proto
    ${CMAKE_CURRENT_SOURCE_DIR}/main_proto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_proto.cpp
)


set(GANTRY_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

foreach(REV IN LISTS REVISIONS)
  foreach(AXIS IN LISTS AXES)
    add_executable(gantry-${AXIS}-${REV}
            ${GANTRY_FW_LINTABLE_SRCS_COMMON}
            ${GANTRY_FW_LINTABLE_SRCS_${REV}}
          ${GANTRY_FW_NON_LINTABLE_SRCS}
          ${CMAKE_SOURCE_DIR}/gantry/firmware/axis_${AXIS}_hardware_config.c)

    target_gantry_core(gantry-${AXIS}-${REV} ${AXIS} ${REV})
    target_ot_motor_control(gantry-${AXIS}-${REV})

    target_link_libraries(gantry-${AXIS}-${REV}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_Gantry STM32G4xx_FreeRTOS_Gantry
            can-core)

    set_target_properties(gantry-${AXIS}-${REV}
            PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED TRUE
            C_STANDARD 11
            C_STANDARD_REQUIRED TRUE)

    target_include_directories(gantry-${AXIS}-${REV}
            PUBLIC ${CMAKE_SOURCE_DIR}/include)

    target_compile_options(gantry-${AXIS}-${REV}
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

    set_target_properties(gantry-${AXIS}-${REV}
            PROPERTIES
            CROSSCOMPILING_EMULATOR
            "${ARM_GDB};--command=${GDBINIT_FILE}")

    add_custom_command(OUTPUT gantry-${AXIS}-${REV}.hex
            COMMAND ${CROSS_OBJCOPY} ARGS gantry-${AXIS}-${REV} "-Oihex" gantry-${AXIS}-${REV}.hex
            DEPENDS gantry-${AXIS}-${REV}
            VERBATIM)
    add_custom_target(gantry-${AXIS}-${REV}-hex ALL
            DEPENDS gantry-${AXIS}-${REV}.hex)

    add_custom_command(OUTPUT gantry-${AXIS}-${REV}.bin
            COMMAND ${CROSS_OBJCOPY} ARGS gantry-${AXIS}-${REV} "-Obinary" gantry-${AXIS}-${REV}.bin
            DEPENDS gantry-${AXIS}-${REV}
            VERBATIM)
    add_custom_target(gantry-${AXIS}-${REV}-bin ALL
            DEPENDS gantry-${AXIS}-${REV}.bin)

    # Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
    # arguable misuse of the concept) to the appropriate cross-gdb with
    # remote-target. You should make sure st-util is running; that's not
    # done here because it won't be multi-os compatible, and also it
    # should be running the entire time and that's tough to accomplish
    # in a custom command
    add_custom_target(gantry-debug-${AXIS}-${REV}
            COMMAND gantry-${AXIS}-${REV}
            USES_TERMINAL
            )

    # Targets to create full image hex file containing both bootloader and application
    add_custom_command(
            OUTPUT gantry-${AXIS}-${REV}-image.hex
            DEPENDS gantry-${AXIS}-${REV}-hex bootloader-gantry-${AXIS}-hex gantry-${AXIS}-${REV}.hex $<TARGET_FILE_DIR:bootloader-gantry-${AXIS}>/bootloader-gantry-${AXIS}.hex
            COMMAND ${CMAKE_SOURCE_DIR}/hex_combine.py gantry-${AXIS}-${REV}-image.hex $<TARGET_FILE_DIR:bootloader-gantry-${AXIS}>/bootloader-gantry-${AXIS}.hex gantry-${AXIS}-${REV}.hex
            VERBATIM)
    add_custom_target(gantry-${AXIS}-${REV}-image-hex ALL
            DEPENDS gantry-${AXIS}-${REV}-image.hex)

    # Targets to flash bootloader and firmware
    add_custom_target(gantry-${AXIS}-${REV}-flash
            COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${DISCO_FILE}" "-c" "program ${CMAKE_CURRENT_BINARY_DIR}/gantry-${AXIS}-${REV}-image.hex;reset;exit"
            VERBATIM
            COMMENT "Flashing gantry ${AXIS} ${REV}"
            DEPENDS gantry-${AXIS}-${REV}-image-hex)
  endforeach()
endforeach()

#default targets
foreach(AXIS IN LISTS AXES)
  add_custom_target(gantry-${AXIS}-image-hex DEPENDS gantry-${AXIS}-${DEFAULT_REVISION}-image-hex)
  add_custom_target(gantry-debug-${AXIS} DEPENDS gantry-debug-${AXIS}-${DEFAULT_REVISION})
  add_custom_target(gantry-${AXIS} DEPENDS gantry-${AXIS}-${DEFAULT_REVISION})
  add_custom_target(gantry-${AXIS}-flash DEPENDS gantry-${AXIS}-${DEFAULT_REVISION}-flash)
endforeach()
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
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${GANTRY_FW_LINTABLE_SRCS_COMMON} ${GANTRY_X_CORE_SRC} ${GANTRY_Y_CORE_SRC} ${GANTRY_FW_LINTABLE_SRCS_rev1} ${GANTRY_FW_LINTABLE_SRCS_proto})
list(APPEND LINT_TARGETS "gantry-lint")
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
