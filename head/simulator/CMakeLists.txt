# Load free for the posix port
find_package(FreeRTOS)

# Set up the include directories
include_directories(
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/include
        ${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix
        ${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils

)

# Create the FreeRTOS source file list
FILE(
        GLOB FREERTOS_SOURCES
        ${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/*.c
)

# Add the posix port files
list(APPEND FREERTOS_SOURCES "${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/portable/MemMang/heap_3.c")
list(APPEND FREERTOS_SOURCES "${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c")
list(APPEND FREERTOS_SOURCES "${FreeRTOS_SOURCE_DIR}/FreeRTOS/Source/portable/ThirdParty/GCC/Posix/port.c")


# Create the freertos lib
add_library(freertos-head STATIC ${FREERTOS_SOURCES})

add_executable(
        head-simulator
        main.cpp
        freertos_idle_timer_task.cpp
        eeprom_keys.cpp
        ${HEAD-CORE-SRC}
)

add_revision(TARGET head-simulator REVISION a1)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
    target_compile_definitions(head-simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(head-simulator PUBLIC ENABLE_LOGGING)

target_can_simlib(head-simulator)

target_i2c_simlib(head-simulator)

target_head_core_proto(head-simulator)

target_ot_motor_control(head-simulator)

target_link_libraries(head-simulator PRIVATE can-core common-simulation freertos-head pthread Boost::boost Boost::program_options state_manager)

set_target_properties(head-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
