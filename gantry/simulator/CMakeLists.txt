# Load free for the posix port
find_package(FreeRTOS)

set(AXES x y)
set(SIM_REV proto)
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
foreach(AXIS IN LISTS AXES)
  add_executable(
          gantry-${AXIS}-simulator
          ${GANTRY_SIMULATOR_SRC}
  )

  if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux" AND DEFINED ENV{USE_SOCKETCAN})
      target_compile_definitions(gantry-${AXIS}-simulator PUBLIC USE_SOCKETCAN)
  endif()

  target_compile_definitions(gantry-${AXIS}-simulator PUBLIC ENABLE_LOGGING)

  target_can_simlib(gantry-${AXIS}-simulator)

  target_gantry_core(gantry-${AXIS}-simulator ${AXIS} ${SIM_REV})

  target_ot_motor_control(gantry-${AXIS}-simulator)

  target_link_libraries(gantry-${AXIS}-simulator PRIVATE can-core common-simulation freertos-gantry pthread Boost::boost)

  set_target_properties(gantry-${AXIS}-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
endforeach()
