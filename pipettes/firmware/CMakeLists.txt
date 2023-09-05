# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)


set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(PIPETTE_FW_LINTABLE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_configurations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utility_configurations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_keys.cpp
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
    ${COMMON_EXECUTABLE_DIR}/gpio.cpp
    ${CAN_FW_DIR}/hal_can.c
    ${CAN_FW_DIR}/hal_can_bus.cpp
    ${CAN_FW_DIR}/utils.c)

set(PIPETTE_FW_NON_LINTABLE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
    ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_timer_hardware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware_config.c
    ${COMMON_EXECUTABLE_DIR}/errors/errors.c
    ${COMMON_EXECUTABLE_DIR}/system/app_update.c
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
    ${COMMON_EXECUTABLE_DIR}/gpio.c
    )


macro(pipettes_loop)
    set(_driver_suffix ${PROJECT_NAME}_${REVISION})

    add_STM32G4_driver(${_driver_suffix})
    add_STM32G4_freertos(${_driver_suffix})

    target_i2c_firmware(${REVISION_TARGET})
    target_ot_motor_control(${REVISION_TARGET})

    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_${_driver_suffix}
            STM32G4xx_FreeRTOS_${_driver_suffix}
            can-core common-core
            )
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
    install_if_latest_revision(
        FILENAME ${REVISION_HEX_IMAGE_FILE}
        COMPONENT pipette-images
        THISREVISION ${REVISION}
        REVISIONS ${REVISIONS})
endmacro()

macro(pipettes_single_loop)
    target_pipettes_core_single(${REVISION_TARGET} "rev1")
    pipettes_loop()
endmacro()

macro(pipettes_multi_loop)
    target_pipettes_core_multi(${REVISION_TARGET} "rev1")
    pipettes_loop()
endmacro()

macro(pipettes_96_loop)
    target_pipettes_core_96(${REVISION_TARGET} "rev1")
    pipettes_loop()
endmacro()

set(pipette_srcs ${PIPETTE_FW_LINTABLE_SRCS}
                 ${PIPETTE_FW_NON_LINTABLE_SRCS}
)

foreach_revision(
    PROJECT_NAME pipettes-single
    REVISIONS b1 c2 d1
    SOURCES pipette_srcs pipette_srcs pipette_srcs
    CALL_FOREACH_REV pipettes_single_loop
    )

foreach_revision(
    PROJECT_NAME pipettes-multi
    REVISIONS b1 c2 d1
    CALL_FOREACH_REV pipettes_multi_loop
    SOURCES pipette_srcs pipette_srcs pipette_srcs
    )

foreach_revision(
    PROJECT_NAME pipettes-96
    REVISIONS b1 c1 d2
    CALL_FOREACH_REV pipettes_96_loop
    SOURCES pipette_srcs pipette_srcs pipette_srcs
    )

alias_for_revision(PROJECT_NAME pipettes-single REVISION b1 REVISION_ALIAS rev1)
alias_for_revision(PROJECT_NAME pipettes-multi REVISION b1 REVISION_ALIAS rev1)
alias_for_revision(PROJECT_NAME pipettes-96 REVISION b1 REVISION_ALIAS rev1)


add_custom_target(pipettes-images DEPENDS pipettes-96-images pipettes-multi-images pipettes-single-images)
add_custom_target(pipettes-exes DEPENDS pipette-96-exes pipettes-multi-exes pipettes-single-exes)
add_custom_target(pipettes-applications DEPENDS pipettes-96-applications pipettes-multi-applications pipettes-single-applications)

add_clang_tidy_target(TARGET_NAME pipettes-lint LINT_SOURCES ${PIPETTE_FW_LINTABLE_SRCS})
