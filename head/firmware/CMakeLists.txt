# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# list of PCBA revisions following EE PLM standards
set(REVISIONS a1 b1 c2)
# base project name
set(PROJECT_NAME head)

# The default revision that you'll get if you use targets not qualified
# by revision (i.e. if you do cmake --build --preset=head --target head
# rather than cmake --build --preset=head --target head-b1)
set(DEFAULT_REVISION c2)

# Add source files that should be checked by clang-tidy here
set(HEAD_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/presence_sensing_hardware.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_keys.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

set(head_a1_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/main_proto.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_proto.c
  ${HEAD_FW_LINTABLE_SRCS}
  ${HEAD_FW_NON_LINTABLE_SRCS}
  )

set(head_b1_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_rev1.c
  ${HEAD_FW_LINTABLE_SRCS}
  ${HEAD_FW_NON_LINTABLE_SRCS}
  )

macro(_loop_body)
  set(_libname_suffix head_${REVISION})
  add_STM32G4_driver(${_libname_suffix})
  add_STM32G4_freertos(${_libname_suffix})

  target_include_directories(STM32G4xx_Drivers_${_libname_suffix}
        PUBLIC .)
  target_include_directories(STM32G4xx_FreeRTOS_${_libname_suffix}
        PUBLIC .)

  target_compile_definitions(STM32G4xx_Drivers_${_libname_suffix}
        PUBLIC STM32G491xx)
  target_compile_definitions(STM32G4xx_FreeRTOS_${_libname_suffix}
        PUBLIC STM32G491xx)

  set_target_properties(
        STM32G4xx_FreeRTOS_${_libname_suffix}
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")


  if (${REVISION} STREQUAL "a1")
    target_head_core_proto(${REVISION_TARGET})
  else()
    target_head_core_rev1(${REVISION_TARGET})
  endif()
  target_ot_motor_control(${REVISION_TARGET})
  target_i2c_firmware(${REVISION_TARGET})

  target_link_libraries(${REVISION_TARGET}
          PUBLIC STM32G491RETx
          STM32G4xx_Drivers_${_libname_suffix} STM32G4xx_FreeRTOS_${_libname_suffix}
          can-core)

  set_target_properties(${REVISION_TARGET}
          PROPERTIES CXX_STANDARD 20
          CXX_STANDARD_REQUIRED TRUE
          C_STANDARD 11
          C_STANDARD_REQUIRED TRUE)
  install_if_latest_revision(
    FILENAME ${REVISION_HEX_IMAGE_FILE}
    COMPONENT robot-images
    THISREVISION ${REVISION}
    REVISIONS ${REVISIONS}
    )
endmacro(_loop_body)

foreach_revision(
  PROJECT_NAME head
  DEFAULT_REVISION c2
  REVISIONS ${REVISIONS}
  SOURCES head_a1_sources head_b1_sources head_b1_sources
  CALL_FOREACH_REV _loop_body)

alias_for_revision(PROJECT_NAME head REVISION a1 REVISION_ALIAS proto)
alias_for_revision(PROJECT_NAME head REVISION b1 REVISION_ALIAS rev1)

set(lint_sources ${HEAD_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
add_clang_tidy_target(TARGET_NAME head-lint LINT_SOURCES ${lint_sources})
