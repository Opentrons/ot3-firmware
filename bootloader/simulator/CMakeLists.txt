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
add_library(freertos-bootloader STATIC ${FREERTOS_SOURCES})

set(BOOTLOADER_SIMULATOR_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/updater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        )

add_executable(
        bootloader-simulator
        ${BOOTLOADER_SIMULATOR_SRC}
        )

add_revision(TARGET bootloader-simulator REVISION a1)
add_revision(TARGET bootloader-simulator REVISION e1)

target_compile_options(
        bootloader-simulator
        PRIVATE
        -Wall
        -Wextra
        -Wno-missing-field-initializers
        -Werror
)

if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
    target_compile_definitions(bootloader-simulator PUBLIC USE_SOCKETCAN)
endif()

target_compile_definitions(bootloader-simulator PUBLIC ENABLE_LOGGING)

target_can_simlib(bootloader-simulator)

target_link_libraries(bootloader-simulator PRIVATE can-core freertos-bootloader pthread bootloader-core common-simulation Boost::boost Boost::program_options)

set_target_properties(bootloader-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
