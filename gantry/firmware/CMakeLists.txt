# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")
set(CAN_FW_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(GANTRY_FW_LINTABLE_SRCS_COMMON
    ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/eeprom_keys.cpp
    ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        )

set(GANTRY_BASE_SOURCES ${GANTRY_FW_NON_LINTABLE_SRCS} ${GANTRY_FW_LINTABLE_SRCS_COMMON})

set(GANTRY_FW_LINTABLE_SRCS_b1
    ${CMAKE_CURRENT_SOURCE_DIR}/main_rev1.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_rev1.cpp
)
set(GANTRY_FW_LINTABLE_SRCS_a1
    ${CMAKE_CURRENT_SOURCE_DIR}/main_proto.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interfaces_proto.cpp
)
set(GANTRY_FW_LINTABLE_SRCS_c1 ${_GANTRY_FW_LINTABLE_SRCS_b1})

set(GANTRY_X_A1_SRCS
    ${GANTRY_BASE_SOURCES}
    ${GANTRY_FW_LINTABLE_SRCS_a1}
    ${CMAKE_SOURCE_DIR}/gantry/firmware/axis_x_proto_hardware_config.c
    )
set(GANTRY_X_B1_SRCS
    ${GANTRY_BASE_SOURCES}
    ${GANTRY_FW_LINTABLE_SRCS_b1}
    ${CMAKE_SOURCE_DIR}/gantry/firmware/axis_x_rev1_hardware_config.c
)
set(GANTRY_X_C1_SRCS ${GANTRY_X_B1_SRCS})
set(GANTRY_X_C2_SRCS ${GANTRY_X_C1_SRCS})

set(GANTRY_Y_A1_SRCS
    ${GANTRY_BASE_SOURCES}
    ${GANTRY_FW_LINTABLE_SRCS_a1}
    ${CMAKE_SOURCE_DIR}/gantry/firmware/axis_y_proto_hardware_config.c
    )
set(GANTRY_Y_B1_SRCS
    ${GANTRY_BASE_SOURCES}
    ${GANTRY_FW_LINTABLE_SRCS_b1}
    ${CMAKE_SOURCE_DIR}/gantry/firmware/axis_y_rev1_hardware_config.c
)
set(GANTRY_Y_C1_SRCS ${GANTRY_Y_B1_SRCS})
set(GANTRY_Y_C2_SRCS ${GANTRY_Y_C1_SRCS})

macro(gantry_loop_internal)
    set(_gli_options)
    set(_gli_multivalue)
    set(_gli_onevalue AXIS)
    cmake_parse_arguments(_gli "${_gli_options}" "${_gli_onevalue}" "${_gli_multivalue}" ${ARGN})
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

    if(${REVISION} STREQUAL "a1")
        set(REV proto)
    else()
        set(REV rev1)
    endif()

    target_gantry_core(${REVISION_TARGET} ${_gli_AXIS} ${REV})
    target_ot_motor_control(${REVISION_TARGET})
    target_i2c_firmware(${REVISION_TARGET})

    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32G491RETx
            STM32G4xx_Drivers_${_driver_suffix} STM32G4xx_FreeRTOS_${_driver_suffix}
            can-core)

    install_if_latest_revision(
        FILENAME ${REVISION_HEX_IMAGE_FILE}
        COMPONENT robot-images
        THISREVISION ${REVISION}
        REVISIONS ${REVISIONS}
    )
endmacro()

macro(gantry_loop_internal_x)
    gantry_loop_internal(AXIS x)
endmacro()

macro(gantry_loop_internal_y)
    gantry_loop_internal(AXIS y)
endmacro()

foreach_revision(
    PROJECT_NAME gantry-x
    CALL_FOREACH_REV gantry_loop_internal_x
    REVISIONS a1 b1 c1 c2
    SOURCES GANTRY_X_A1_SRCS GANTRY_X_B1_SRCS GANTRY_X_C1_SRCS GANTRY_X_C2_SRCS
    )
foreach_revision(
    PROJECT_NAME gantry-y
    CALL_FOREACH_REV gantry_loop_internal_y
    REVISIONS a1 b1 c1 c2
    SOURCES GANTRY_Y_A1_SRCS GANTRY_Y_B1_SRCS GANTRY_Y_C1_SRCS GANTRY_Y_C2_SRCS
    )


add_custom_target(gantry-images DEPENDS gantry-x-images gantry-y-images)
add_custom_target(gantry-exes DEPENDS gantry-x-exes gantry-y-exes)
add_custom_target(gantry-applications DEPENDS gantry-x-applications gantry-y-applications)

alias_for_revision(PROJECT_NAME gantry-x REVISION a1 REVISION_ALIAS proto)
alias_for_revision(PROJECT_NAME gantry-y REVISION a1 REVISION_ALIAS proto)

alias_for_revision(PROJECT_NAME gantry-x REVISION b1 REVISION_ALIAS rev1)
alias_for_revision(PROJECT_NAME gantry-y REVISION b1 REVISION_ALIAS rev1)

add_clang_tidy_target(
    TARGET_NAME gantry-lint
    LINT_SOURCES ${GANTRY_FW_LINTABLE_SRCS_COMMON}
                 ${GANTRY_FW_LINTABLE_SRCS_b1}
                 ${GANTRY_FW_LINTABLE_SRCS_a1})
