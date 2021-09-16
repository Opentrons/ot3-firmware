# This CMakeLists.txt handles compiling all the parts of the pipettes
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

set(COMMON_CORE_DIR "${CMAKE_SOURCE_DIR}/common/core")

add_library(pipettes-core STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
  )

set_target_properties(pipettes-core
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED TRUE)

target_link_libraries(pipettes-core PUBLIC STM32G4xx_FreeRTOS_Pipettes)


target_include_directories(pipettes-core
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_compile_options(pipettes-core
  PRIVATE
  -Wall
  -Werror
  -Weffc++
  -Wreorder
  -Wsign-promo
  -Wextra-semi
  -Wctor-dtor-privacy
  -fno-rtti
  )