# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")
set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

set(REVISIONS hepa-rev1)

# Add source files that should be checked by clang-tidy here
set(HEPA_UV_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/led_control_task/led_control_hardware.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/hepa_control_hardware.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        ${CAN_FW_DIR}/hal_can_bus.cpp
        ${CAN_FW_DIR}/utils.c
        ${CAN_FW_DIR}/hal_can.c
        )

set(HEPAUV_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hepauv_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

set(HEPAUV_SRCS_B1
        ${HEPA_UV_FW_LINTABLE_SRCS}
        ${HEPAUV_FW_NON_LINTABLE_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
)
set(HEPAUV_SRCS_C1 ${HEPAUV_SRCS_B1})
set(HEPAUV_SRCS_C2 ${HEPAUV_SRCS_C1})

macro(hepa_uv_loop)
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

    target_hepa_uv_core(${REVISION_TARGET})
    target_ot_motor_control(${REVISION_TARGET})

    target_i2c_firmware(${REVISION_TARGET})

    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_${_driver_suffix} STM32G4xx_FreeRTOS_${_driver_suffix}
            can-core)
    install_if_latest_revision(
        FILENAME ${REVISION_HEX_IMAGE_FILE}
        COMPONENT hepauv-images
        THISREVISION ${REVISION}
        REVISIONS ${REVISIONS}
    )
endmacro()

foreach_revision(
    PROJECT_NAME hepa-uv
    REVISIONS b1
    SOURCES HEPAUV_SRCS_B1
    CALL_FOREACH_REV hepa_uv_loop)

alias_for_revision(PROJECT_NAME hepa-uv REVISION b1 REVISION_ALIAS rev1)

add_clang_tidy_target(
    TARGET_NAME hepa-uv-lint
    LINT_SOURCES ${HEPA_UV_FW_LINTABLE_SRCS})
