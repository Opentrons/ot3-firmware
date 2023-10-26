# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")
set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

set(REVISIONS gripper-proto gripper-rev1)

set(DEFAULT_REVISION gripper-proto)

# Add source files that should be checked by clang-tidy here
set(GRIPPER_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_grip_motor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_z_motor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_keys.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_g.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_z.c
        ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware_shared.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

set(GRIPPER_SRCS_A1
        ${GRIPPER_FW_LINTABLE_SRCS}
        ${GRIPPER_FW_NON_LINTABLE_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/main_proto.cpp
        )
set(GRIPPER_SRCS_B1
        ${GRIPPER_FW_LINTABLE_SRCS}
        ${GRIPPER_FW_NON_LINTABLE_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
)
set(GRIPPER_SRCS_C1 ${GRIPPER_SRCS_B1})
set(GRIPPER_SRCS_C2 ${GRIPPER_SRCS_C1})

macro(gripper_loop)
    set(_driver_suffix ${PROJECT_NAME}_${REVISION})
    add_STM32G4_driver(${_driver_suffix})
    add_STM32G4_freertos(${_driver_suffix})

    target_include_directories(STM32G4xx_Drivers_${_driver_suffix}
        PUBLIC .)
    target_include_directories(STM32G4xx_FreeRTOS_${_driver_suffix}
        PUBLIC .)

    target_compile_definitions(STM32G4xx_Drivers_${_driver_suffix}
        PUBLIC STM32G491xx)
    target_compile_definitions(STM32G4xx_FreeRTOS_${_driver_suffix}
        PUBLIC STM32G491xx)

    set_target_properties(
        STM32G4xx_FreeRTOS_${_driver_suffix}
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")

    target_gripper_core(${REVISION_TARGET})
    target_ot_motor_control(${REVISION_TARGET})

    target_i2c_firmware(${REVISION_TARGET})

    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_${_driver_suffix} STM32G4xx_FreeRTOS_${_driver_suffix}
            can-core)
    install_if_latest_revision(
        FILENAME ${REVISION_HEX_IMAGE_FILE}
        COMPONENT gripper-images
        THISREVISION ${REVISION}
        REVISIONS ${REVISIONS}
    )
endmacro()

foreach_revision(
    PROJECT_NAME gripper
    REVISIONS a1 b1 c1 c2
    SOURCES GRIPPER_SRCS_A1 GRIPPER_SRCS_B1 GRIPPER_SRCS_C1 GRIPPER_SRCS_C2
    CALL_FOREACH_REV gripper_loop)

alias_for_revision(PROJECT_NAME gripper REVISION a1 REVISION_ALIAS proto)
alias_for_revision(PROJECT_NAME gripper REVISION b1 REVISION_ALIAS rev1)

add_clang_tidy_target(
    TARGET_NAME gripper-lint
    LINT_SOURCES ${GRIPPER_FW_LINTABLE_SRCS})
