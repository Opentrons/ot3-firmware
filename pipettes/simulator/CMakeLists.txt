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

function(add_simulator_executable TARGET)
    add_executable(
            ${TARGET}
            ${PIPETTES_SIMULATOR_SRC}
    )

    if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
        target_compile_definitions(${TARGET} PUBLIC USE_SOCKETCAN)
    endif()

    target_compile_definitions(${TARGET} PUBLIC ENABLE_LOGGING)


    target_can_simlib(${TARGET})
    target_ot_motor_control(${TARGET})


    if(${TARGET} STREQUAL "pipettes-single")
        target_pipettes_core_single(${TARGET})
    elseif(${TARGET} STREQUAL "pipettes-multi")
        target_pipettes_core_multi(${TARGET})
    elseif(${TARGET} STREQUAL "pipettes-96")
        target_pipettes_core_96(${TARGET})
    elseif(${TARGET} STREQUAL "pipettes-384")
        target_pipettes_core_384(${TARGET})
    else ()
        message(STATUS "Falling back to single channel pipette.")
        target_pipettes_core_single(${TARGET})
    endif ()

    target_link_libraries(${TARGET} PRIVATE can-core common-simulation freertos-pipettes pthread Boost::boost)

    # Set up the include directories
    target_include_directories(
            ${TARGET}
            PUBLIC ${CMAKE_SOURCE_DIR}/include)

    set_target_properties(${TARGET}
            PROPERTIES CXX_STANDARD 20
            CXX_STANDARD_REQUIRED TRUE
            C_STANDARD 11
            C_STANDARD_REQUIRED TRUE)
endfunction()

add_simulator_executable(pipettes-single)
add_simulator_executable(pipettes-multi)
add_simulator_executable(pipettes-96)
add_simulator_executable(pipettes-384)
