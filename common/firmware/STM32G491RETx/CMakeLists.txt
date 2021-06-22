
find_package(GDBSVDTools)
# Fills in the template with values specified by the find_package(OpenOCD) call above
configure_file(./gdbinit.template ./gdbinit)

add_library(
    STM32G491RETx STATIC
    ./startup_stm32g491xx.s
)

target_link_options(STM32G491RETx
  INTERFACE
  "LINKER:-T,$./STM32G491RETx_FLASH.ld"
  "LINKER:--print-memory-usage"
  "LINKER:--error-unresolved-symbols")
