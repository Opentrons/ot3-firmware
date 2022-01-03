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
add_library(freertos-pipettes STATIC ${FREERTOS_SOURCES})

set(PIPETTES_SIMULATOR_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        )

add_executable(
        pipettes_simulator
        ${PIPETTES_SIMULATOR_SRC}
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
    target_compile_definitions(pipettes_simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(pipettes_simulator PUBLIC ENABLE_LOGGING)

target_can_simlib(pipettes_simulator)

target_pipettes_core(pipettes_simulator)

target_ot_motor_control(pipettes_simulator)

target_link_libraries(pipettes_simulator PRIVATE can-core common-simulation freertos-pipettes pthread)


# Set up the include directories
target_include_directories(
        pipettes_simulator
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


set_target_properties(pipettes_simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

