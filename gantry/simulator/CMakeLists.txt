# Load free for the posix port
find_package(FreeRTOS)

# Set up the include directories
include_directories(
        .
        ../firmware
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
add_library(freertos-gantry STATIC ${FREERTOS_SOURCES})

set(GANTRY_SIMULATOR_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
        ../firmware/freertos_idle_timer_task.cpp
        )

add_executable(
        gantry_x_simulator
        ${GANTRY_SIMULATOR_SRC}
)

add_executable(
        gantry_y_simulator
        ${GANTRY_SIMULATOR_SRC}
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_compile_definitions(gantry_x_simulator PUBLIC USE_SOCKETCAN)
    target_compile_definitions(gantry_y_simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(gantry_x_simulator PUBLIC ENABLE_LOGGING)
target_compile_definitions(gantry_y_simulator PUBLIC ENABLE_LOGGING)

target_gantry_core_x(gantry_x_simulator)
target_gantry_core_y(gantry_y_simulator)

target_link_libraries(gantry_x_simulator PRIVATE can-core common-simulation freertos-gantry pthread)
target_link_libraries(gantry_y_simulator PRIVATE can-core common-simulation freertos-gantry pthread)

set_target_properties(gantry_x_simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

set_target_properties(gantry_y_simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
