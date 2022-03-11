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
add_library(freertos-gantry STATIC ${FREERTOS_SOURCES})

set(GANTRY_SIMULATOR_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        )

add_executable(
        gantry-x-simulator
        ${GANTRY_SIMULATOR_SRC}
)

add_executable(
        gantry-y-simulator
        ${GANTRY_SIMULATOR_SRC}
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
    target_compile_definitions(gantry-x-simulator PUBLIC USE_SOCKETCAN)
    target_compile_definitions(gantry-y-simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(gantry-x-simulator PUBLIC ENABLE_LOGGING)
target_compile_definitions(gantry-y-simulator PUBLIC ENABLE_LOGGING)

target_can_simlib(gantry-x-simulator)
target_can_simlib(gantry-y-simulator)

target_gantry_core_x(gantry-x-simulator)
target_gantry_core_y(gantry-y-simulator)

target_ot_motor_control(gantry-x-simulator)
target_ot_motor_control(gantry-y-simulator)

target_link_libraries(gantry-x-simulator PRIVATE can-core common-simulation freertos-gantry pthread Boost::boost)
target_link_libraries(gantry-y-simulator PRIVATE can-core common-simulation freertos-gantry pthread Boost::boost)

set_target_properties(gantry-x-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

set_target_properties(gantry-y-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
