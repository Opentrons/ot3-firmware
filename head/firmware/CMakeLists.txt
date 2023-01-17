# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Head")
add_STM32G4_freertos("Head")

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

# List of revisions of the head that we support
set(REVISIONS head-proto head-rev1)
# The default revision that you'll get if you use targets not qualified
# by revision (i.e. if you do cmake --build --preset=head --target head
# rather than cmake --build --preset=head --target head-proto)
set(DEFAULT_REVISION head-proto)

# Add source files that should be checked by clang-tidy here
set(HEAD_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/presence_sensing_hardware.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        ${CAN_FW_DIR}/hal_can.c
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/utils.c
        )

set(HEAD_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_hardware.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

add_executable(head-proto
        ${CMAKE_CURRENT_SOURCE_DIR}/main_proto.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_proto.c
        ${HEAD_FW_LINTABLE_SRCS}
        ${HEAD_FW_NON_LINTABLE_SRCS})
target_head_core_proto(head-proto)

add_executable(head-rev1
        ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_rev1.c
        ${HEAD_FW_LINTABLE_SRCS}
        ${HEAD_FW_NON_LINTABLE_SRCS})
target_head_core_rev1(head-rev1)
set(IMAGES)
set(EXES)

foreach(REVISION_TARGET IN LISTS REVISIONS )
  target_ot_motor_control(${REVISION_TARGET})

  target_link_libraries(${REVISION_TARGET}
          PUBLIC STM32G491RETx
          STM32G4xx_Drivers_Head STM32G4xx_FreeRTOS_Head
          can-core)

  set_target_properties(${REVISION_TARGET}
          PROPERTIES CXX_STANDARD 20
          CXX_STANDARD_REQUIRED TRUE
          C_STANDARD 11
          C_STANDARD_REQUIRED TRUE)

  target_include_directories(${REVISION_TARGET}
          PUBLIC ${CMAKE_SOURCE_DIR}/include)

  target_compile_options(${REVISION_TARGET}
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
  # Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
  # this dir
  set_target_properties(${REVISION_TARGET}
          PROPERTIES
          CROSSCOMPILING_EMULATOR
          "${ARM_GDB};--command=${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")
  list(APPEND EXES ${REVISION_TARGET})

  add_custom_command(OUTPUT ${REVISION_TARGET}.hex
          COMMAND ${CROSS_OBJCOPY} ARGS ${REVISION_TARGET} "-Oihex" ${REVISION_TARGET}.hex
          DEPENDS ${REVISION_TARGET}
          VERBATIM)

  add_custom_target(${REVISION_TARGET}-hex ALL
          DEPENDS ${REVISION_TARGET}.hex)
  list(APPEND IMAGES ${REVISION_TARGET}-hex)

  add_custom_command(OUTPUT ${REVISION_TARGET}.bin
          COMMAND ${CROSS_OBJCOPY} ARGS ${REVISION_TARGET} "-Obinary" ${REVISION_TARGET}.bin
          DEPENDS ${REVISION_TARGET}
          VERBATIM)

  add_custom_target(${REVISION_TARGET}-bin ALL
          DEPENDS ${REVISION_TARGET}.bin)

  # Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
  # arguable misuse of the concept) to the appropriate cross-gdb with
  # remote-target. You should make sure st-util is running; that's not
  # done here because it won't be multi-os compatible, and also it
  # should be running the entire time and that's tough to accomplish
  # in a custom command
  add_custom_target(${REVISION_TARGET}-debug
          COMMAND ${REVISION_TARGET}
          USES_TERMINAL
          )

  # Targets to create full image hex file containing both bootloader and application
  add_custom_command(
          OUTPUT ${REVISION_TARGET}-image.hex
          DEPENDS ${REVISION_TARGET}-hex bootloader-head-hex ${REVISION_TARGET}.hex $<TARGET_FILE_DIR:bootloader-head>/bootloader-head.hex
          COMMAND ${CMAKE_SOURCE_DIR}/hex_combine.py ${REVISION_TARGET}-image.hex $<TARGET_FILE_DIR:bootloader-head>/bootloader-head.hex ${REVISION_TARGET}.hex
          VERBATIM)
  add_custom_target(${REVISION_TARGET}-image-hex ALL
          DEPENDS ${REVISION_TARGET}-image.hex)

  # Targets to flash bootloader and firmware
  add_custom_target(${REVISION_TARGET}-flash
          COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${DISCO_FILE}" "-c" "program ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-image.hex;reset;exit"
          VERBATIM
          COMMENT "Flashing board"
          DEPENDS ${REVISION_TARGET}-image-hex)
endforeach()

# Default targets
add_custom_target(head-debug DEPENDS ${DEFAULT_REVISION}-debug)
add_custom_target(head
  ${CMAKE_COMMAND} -E copy ${DEFAULT_REVISION} head
  DEPENDS ${DEFAULT_REVISION})
add_custom_command(OUTPUT head-image.hex
  COMMAND ${CMAKE_COMMAND} -E copy ${DEFAULT_REVISION}-image.hex head-image.hex
  DEPENDS ${DEFAULT_REVISION}-image-hex
)
add_custom_target(head-image-hex DEPENDS head-image.hex)
add_custom_target(head-flash DEPENDS ${DEFAULT_REVISION}-flash)
add_custom_target(head-exes DEPENDS head ${EXES})
add_custom_target(head-images DEPENDS head-image-hex ${IMAGES})

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
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} "--extra-arg=-DOPENTRONS_CLANG_TIDY_WORKAROUND_44178" ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${HEAD_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
list(APPEND LINT_TARGETS head-lint)
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
