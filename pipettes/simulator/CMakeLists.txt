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
        pipettes-simulator
        ${PIPETTES_SIMULATOR_SRC}
)


if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
    target_compile_definitions(pipettes-simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(pipettes-simulator PUBLIC ENABLE_LOGGING)


target_can_simlib(pipettes-simulator)
target_ot_motor_control(pipettes-simulator)


if(ENV{PIPETTE_TYPE} STREQUAL "single")
    target_pipettes_core_single(pipettes-simulator)
elseif(ENV{PIPETTE_TYPE} STREQUAL "multi")
    target_pipettes_core_multi(pipettes-simulator)
elseif(ENV{PIPETTE_TYPE} STREQUAL "ninety-six")
    target_pipettes_core_96(pipettes-simulator)
elseif(ENV{PIPETTE_TYPE} STREQUAL "three-eighty-four")
    target_pipettes_core_384(pipettes-simulator)
else ()
    message(STATUS "No pipette type provided. Falling back to single channel.")
    target_pipettes_core_single(pipettes-simulator)
endif ()



target_link_libraries(pipettes-simulator PRIVATE can-core common-simulation freertos-pipettes pthread Boost::boost)


# Set up the include directories
target_include_directories(
        pipettes-simulator
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


set_target_properties(pipettes-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

