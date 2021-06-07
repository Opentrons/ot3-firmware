# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

find_package(STM32G4xx
  COMPONENTS Drivers FreeRTOS)
find_package(CrossGCC)
find_package(OpenOCD)

# Add source files that should be checked by clang-tidy here
set(SYSTEM_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/)

add_library(system
  STATIC
  ${SYSTEM_SRCS})

set_target_properties(system
  PROPERTIES LINKER_LANGUAGE C)

target_link_options(system
  PUBLIC
  "LINKER:-T,${CMAKE_CURRENT_SOURCE_DIR}/STM32G491RETx_FLASH.ld"
  "LINKER:--print-memory-usage"
  "LINKER:--error-unresolved-symbols")

# Incurs at least a relink when you change the linker file (and a recompile of main
# but hopefully that's quick)
set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/main.cpp
PROPERTIES
OBJECT_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/STM32G491RETx_FLASH.ld")


target_include_directories(system PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_compile_options(system
  PUBLIC
  -Wall
  -Werror
  $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
  $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
  $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
  $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)


target_link_libraries(
  system
  STM32G4xx_Drivers STM32G4xx_FreeRTOS)


target_include_directories(STM32G4xx_Drivers
  PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS
  PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers
  PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS
  PUBLIC STM32G491xx)

set_target_properties(
	STM32G4xx_FreeRTOS
  PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")

find_package(GDBSVDTools)

# Fills in the template with values specified by the find_package(OpenOCD) call above
configure_file(./gdbinit.template ./gdbinit)


find_program(ARM_GDB
  arm-none-eabi-gdb-py
  PATHS "${CrossGCC_BINDIR}"
  NO_DEFAULT_PATH
  REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")
# Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
# this dir
set_target_properties(system
  PROPERTIES
  CROSSCOMPILING_EMULATOR
  "${ARM_GDB}")

# set_target_properties(pipettes
#   PROPERTIES
#   CROSSCOMPILING_EMULATOR
#   "${ARM_GDB};--command=${CMAKE_CURRENT_BINARY_DIR}/gdbinit")

