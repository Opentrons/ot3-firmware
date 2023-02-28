# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

find_package(STM32L5xx)
include(RevisionHandling)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")

set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")
set(IMAGES)

# Add source files that should be checked by clang-tidy here
set(PIPETTE_FW_LINTABLE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_configurations.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
    ${COMMON_EXECUTABLE_DIR}/gpio.cpp
    ${CAN_FW_DIR}/hal_can.c
    ${CAN_FW_DIR}/hal_can_bus.cpp
    ${CAN_FW_DIR}/utils.c)

set(PIPETTE_FW_NON_LINTABLE_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32l5xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32l5xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
    ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/can.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_encoder_hardware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_hardware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_timer_hardware.c
    ${CMAKE_CURRENT_SOURCE_DIR}/hardware_config.c
    ${COMMON_EXECUTABLE_DIR}/errors/errors.c
    ${COMMON_EXECUTABLE_DIR}/system/app_update.c
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
    ${COMMON_EXECUTABLE_DIR}/gpio.c
    )


macro(pipettes_l5_loop)
    set(_driver_suffix ${PROJECT_NAME}_${REVISION})
    add_STM32L5_driver(${_driver_suffix})
    add_STM32L5_freertos(${_driver_suffix})


    target_include_directories(STM32L5xx_Drivers_${_driver_suffix}
        PUBLIC .)
    target_include_directories(STM32L5xx_FreeRTOS_${_driver_suffix}
        PUBLIC .)

    target_compile_definitions(STM32L5xx_Drivers_${_driver_suffix}
        PUBLIC STM32L562xx)
    target_compile_definitions(STM32L5xx_FreeRTOS_${_driver_suffix}
        PUBLIC STM32L562xx)

    set_target_properties(
        STM32L5xx_FreeRTOS_${_driver_suffix}
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")
    target_i2c_firmware(${REVISION_TARGET})
    target_ot_motor_control(${REVISION_TARGET})
    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32L562RETx
            STM32L5xx_Drivers_${_driver_suffix} STM32L5xx_FreeRTOS_${_driver_suffix}
            can-core common-core
            )

endmacro()

macro(pipettes_single_l5_loop)
    target_pipettes_core_single(${REVISION_TARGET} "proto")
    pipettes_l5_loop()
endmacro()

macro(pipettes_multi_l5_loop)
    target_pipettes_core_multi(${REVISION_TARGET} "proto")
    pipettes_l5_loop()
endmacro()

macro(pipettes_96_l5_loop)
    target_pipettes_core_96(${REVISION_TARGET}  "proto")
    pipettes_l5_loop()
endmacro()

set(_srcs ${PIPETTE_FW_LINTABLE_SRCS} ${PIPETTE_FW_NON_LINTABLE_SRCS})

foreach_revision(
    PROJECT_NAME pipettes-single
    REVISIONS a1
    SOURCES _srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_single_l5_loop
    )

foreach_revision(
    PROJECT_NAME pipettes-multi
    REVISIONS a1
    SOURCES _srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_multi_l5_loop
    )

foreach_revision(
    PROJECT_NAME pipettes-96
    REVISIONS a1
    SOURCES _srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_96_l5_loop
    )

add_custom_target(pipettes-images DEPENDS pipettes-single-images pipettes-multi-images pipettes-96-images)
add_custom_target(pipettes-applications DEPENDS pipettes-single-applications pipettes-multi-applications pipettes-96-applications)
add_custom_target(pipettes-exes DEPENDS pipettes-single-exes pipettes-multi-exes pipettes-96-exes)
