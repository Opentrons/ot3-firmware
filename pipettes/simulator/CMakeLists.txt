# Load free for the posix port
find_package(FreeRTOS)

# Set up the include directories
set(PIPETTE_SIM_INCLUDE
        .
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_BINARY_DIR}/generated/
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
target_include_directories(freertos-pipettes PUBLIC ${PIPETTE_SIM_INCLUDE})

function(add_simulator_executable TARGET)
    add_executable(
            ${TARGET}
            ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/motor_configurations.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/interfaces.cpp
            )
    target_include_directories(${TARGET} PUBLIC ${PIPETTE_SIM_INCLUDE})
    target_i2c_simlib(${TARGET})


    if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
        target_compile_definitions(${TARGET} PUBLIC USE_SOCKETCAN)
    endif()

    target_compile_definitions(${TARGET} PUBLIC ENABLE_LOGGING)


    target_can_simlib(${TARGET})
    target_ot_motor_control(${TARGET})

    if(${TARGET} STREQUAL "pipettes-single-simulator")
        target_pipettes_core_single(${TARGET} "proto")
    elseif(${TARGET} STREQUAL "pipettes-multi-simulator")
        target_pipettes_core_multi(${TARGET} "proto")
    elseif(${TARGET} STREQUAL "pipettes-96-simulator")
        target_pipettes_core_96(${TARGET} "proto")
    elseif(${TARGET} STREQUAL "pipettes-384-simulator")
        target_pipettes_core_384(${TARGET} "proto")
    else ()
        message(FATAL_ERROR "Invalid pipette target provided.")
    endif ()

    target_link_libraries(${TARGET} PRIVATE can-core common-simulation freertos-pipettes pthread Boost::boost Boost::program_options)

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

add_simulator_executable(pipettes-single-simulator)
add_simulator_executable(pipettes-multi-simulator)
add_simulator_executable(pipettes-96-simulator)
add_simulator_executable(pipettes-384-simulator)
