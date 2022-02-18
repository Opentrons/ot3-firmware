add_library(
    STM32G491RETx_bootloader STATIC
    ./startup_stm32g491xx.s
)

target_link_options(STM32G491RETx_bootloader
  INTERFACE
  "LINKER:-T,${CMAKE_CURRENT_SOURCE_DIR}/STM32G491RETx_FLASH.ld"
  "LINKER:--print-memory-usage"
  "LINKER:--gc-sections"
  "LINKER:--error-unresolved-symbols")

# Incurs at least a relink when you change the linker file (and a recompile of main
# but hopefully that's quick)
set_source_files_properties(./startup_stm32g491xx.s
  PROPERTIES
  OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/STM32G491RETx_FLASH.ld)
