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
add_library(freertos STATIC ${FREERTOS_SOURCES})

add_executable(
        head-simulator
        main.cpp
        ../firmware/freertos_idle_timer_task.cpp
        ${HEAD-CORE-SRC}
)

target_link_libraries(head-simulator PRIVATE can-core freertos pthread)

set_target_properties(head-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
