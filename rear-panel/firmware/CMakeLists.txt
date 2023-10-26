# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)
include(ClangTidy)

find_program(ARM_GDB
        arm-none-eabi-gdb-py
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)
message(STATUS "Found svd exe at ${GDBSVDTools_gdbsvd_EXECUTABLE}")

find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
        PATHS "${CrossGCC_BINDIR}"
        NO_DEFAULT_PATH
        REQUIRED)

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")
set(COMMON_MCU_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../startup/STM32G491")

# List of revisions of the rear-panel that we support
set(REVISIONS rear-panel-rev1)
# The default revision that you'll get if you use targets not qualified
# by revision (i.e. if you do cmake --build --preset=rear-panel --target rear-panel
# rather than cmake --build --preset=rear-panel --target rear-panel-proto)
set(DEFAULT_REVISION rear-panel-rev1)

# Add source files that should be checked by clang-tidy here
set(REAR_PANEL_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/host_comms_task/freertos_comms_task.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/light_control_task/light_control_hardware.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        ${COMMON_EXECUTABLE_DIR}/gpio.cpp
        )

set(REAR_PANEL_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${CMAKE_CURRENT_SOURCE_DIR}/led_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/system_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/host_comms_task/usbd_conf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/host_comms_task/usb_hardware.c
        ${CMAKE_CURRENT_SOURCE_DIR}/host_comms_task/usbd_desc.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${COMMON_EXECUTABLE_DIR}/gpio.c
        )


find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
  PATHS "${CrossGCC_BINDIR}"
  NO_DEFAULT_PATH
  REQUIRED)

set(rearpanel_b1_srcs ${REAR_PANEL_FW_LINTABLE_SRCS}
                      ${REAR_PANEL_FW_NON_LINTABLE_SRCS}
                      ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

# Incurs at least a relink when you change the linker file (and a recompile of main
# but hopefully that's quick)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    PROPERTIES
    OBJECT_DEPENDS "${COMMON_MCU_DIR}/STM32G491RETx_FLASH.ld")


macro(rearpanel_loop)
  set(_driver_suffix ${PROJECT_NAME}_${REVISION})
  add_STM32G4_driver(${_driver_suffix})
  add_STM32G4_freertos(${_driver_suffix})
  add_STM32G4_usb(${_driver_suffix})
  target_include_directories(STM32G4xx_Drivers_${_driver_suffix}
        PUBLIC .)
  target_include_directories(STM32G4xx_FreeRTOS_${_driver_suffix}
        PUBLIC .)
  target_include_directories(STM32G4xx_USB_${_driver_suffix}
        PUBLIC set(COMMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/host_comms_task"))

  target_compile_definitions(STM32G4xx_Drivers_${_driver_suffix}
        PUBLIC STM32G491xx)
  target_compile_definitions(STM32G4xx_FreeRTOS_${_driver_suffix}
        PUBLIC STM32G491xx)
  target_compile_definitions(STM32G4xx_USB_${_driver_suffix}
        PUBLIC STM32G491xx)

  set_target_properties(
        STM32G4xx_FreeRTOS_${_driver_suffix}
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")
  target_rear_panel_core_rev1(${REVISION_TARGET})
  target_i2c_firmware(${REVISION_TARGET})

  target_link_libraries(${REVISION_TARGET}
          PUBLIC
          STM32G4xx_Drivers_${_driver_suffix}
          STM32G4xx_FreeRTOS_${_driver_suffix}
          STM32G4xx_USB_${_driver_suffix}
          rear-panel-startup-STM32G491
          )

  set_target_properties(${REVISION_TARGET}
          PROPERTIES CXX_STANDARD 20
          CXX_STANDARD_REQUIRED TRUE
          C_STANDARD 11
          C_STANDARD_REQUIRED TRUE)

  target_include_directories(${REVISION_TARGET}
          PUBLIC ${CMAKE_SOURCE_DIR}/include)

  target_compile_options(${REVISION_TARGET}
          PUBLIC
          -Wall
          -Werror
          -Wextra
          -Wno-missing-field-initializers
          $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
          $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
          $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
          $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
          $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
          $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

    # Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
    # this dir
    set(GDBINIT_FILE "${CMAKE_BINARY_DIR}/rear-panel/startup/STM32G491/gdbinit")
    set_target_properties(${REVISION_TARGET}
            PROPERTIES
            CROSSCOMPILING_EMULATOR
            "${ARM_GDB};--command=${GDBINIT_FILE}")

    # Calculate and package the integrity info for this module

  add_custom_target(${REVISION_TARGET}-integrity ALL
    COMMAND ${CROSS_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET} -Obinary
        ${REVISION_TARGET}-scratch.bin --gap-fill=0xFF
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/calculate_checksum.py
        ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-scratch.bin
        ${REVISION_TARGET}
        "0x08008400"
        ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-integrity-info
    COMMAND ${CROSS_OBJCOPY} ${REVISION_TARGET}
        --update-section
        .integrity_info=${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-integrity-info
    DEPENDS ${REVISION_TARGET}
    DEPENDS ${CMAKE_SOURCE_DIR}/calculate_checksum.py
  )

  # The .hex target depends on the module's integrity-target in order to generate
  # the integrity information expected by the startup app. Therefore, all other
  # flashable targets should derive from the initial hex.
  add_custom_command(OUTPUT ${REVISION_TARGET}.hex
    COMMAND ${CROSS_OBJCOPY} ARGS ${REVISION_TARGET} "-Oihex" ${REVISION_TARGET}.hex
    DEPENDS ${REVISION_TARGET}
    DEPENDS ${REVISION_TARGET}-integrity
    DEPENDS ${CMAKE_SOURCE_DIR}/calculate_checksum.py
    VERBATIM)

  add_custom_target(${REVISION_TARGET}-hex ALL
    DEPENDS ${REVISION_TARGET}.hex)

  add_custom_command(OUTPUT ${REVISION_TARGET}.bin
    COMMAND ${CROSS_OBJCOPY} ARGS
      ${REVISION_TARGET}.hex
      "-Iihex" "-Obinary" "--gap-fill=0xFF"
      ${REVISION_TARGET}.bin
    DEPENDS ${REVISION_TARGET}.hex
    VERBATIM)
  add_custom_target(${REVISION_TARGET}-bin ALL
    DEPENDS ${REVISION_TARGET}.bin)
  add_dependencies(${REVISION_TARGET}-bin ${REVISION_TARGET}-hex)
  add_custom_target(${REVISION_TARGET}-debug
    COMMENT "Starting gdb and openocd"
    COMMAND ${REVISION_TARGET}
    DEPENDS ${REVISION_TARGET}-integrity
    USES_TERMINAL
    )
  set(HEX_IMG_FILENAME ${REVISION_TARGET}-image.hex)
  set(HEX_IMG_NAME "${CMAKE_CURRENT_BINARY_DIR}/${HEX_IMG_FILENAME}")

  set(BIN_IMG_NAME "${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-image.bin")

  set(STARTUP_NAME ${REVISION_TARGET}-startup)

  stm32g491_clear_target(${REVISION_TARGET}-clear)

  stm32g491_startup(${STARTUP_NAME} ${REVISION_TARGET})

  # Targets to create full image hex file containing both bootloader and application
  add_custom_command(
      OUTPUT ${HEX_IMG_NAME}
      DEPENDS ${REVISION_TARGET}-hex
      DEPENDS ${STARTUP_NAME}-hex
      DEPENDS ${REVISION_TARGET}.hex
      COMMAND ${CMAKE_SOURCE_DIR}/scripts/hex_combine.py ${HEX_IMG_NAME} $<TARGET_FILE_DIR:${STARTUP_NAME}>/${STARTUP_NAME}.hex ${REVISION_TARGET}.hex
      VERBATIM)
  add_custom_target(${REVISION_TARGET}-image-hex ALL
      DEPENDS ${HEX_IMG_NAME})
  add_dependencies(${REVISION_TARGET}-image-hex
      ${REVISION_TARGET}-hex)

  add_custom_command(OUTPUT "${BIN_IMG_NAME}"
      COMMAND ${CROSS_OBJCOPY} ARGS
          "-Iihex" "-Obinary" "--gap-fill=0xFF"
          "${HEX_IMG_NAME}" "${BIN_IMG_NAME}"
      DEPENDS "${HEX_IMG_NAME}"
      DEPENDS ${REVISION_TARGET}-image-hex
      VERBATIM)
  add_custom_target(${REVISION_TARGET}-image-bin ALL
      DEPENDS ${BIN_IMG_NAME})
  add_dependencies(${REVISION_TARGET}-image-bin
      ${REVISION_TARGET}-image-hex)

   # Runs openocd to flash the board (without using a debugger)
  add_custom_target(${REVISION_TARGET}-flash
      COMMAND "${OpenOCD_EXECUTABLE}"
          "-f" "${COMMON_MCU_DIR}/stm32g4discovery.cfg"
          "-c" "program ${REVISION_TARGET}.hex reset;exit"
      VERBATIM
      COMMENT "Flashing board"
      DEPENDS ${REVISION_TARGET}.hex)
  # Runs openocd to flash the board with the full image (startup included)
  add_custom_target(${REVISION_TARGET}-image-flash
      COMMAND "${OpenOCD_EXECUTABLE}"
          "-f" "${COMMON_MCU_DIR}/stm32g4discovery.cfg"
          "-c" "program ${HEX_IMG_NAME} reset;exit"
      VERBATIM
      COMMENT "Flashing board"
      DEPENDS ${HEX_IMG_NAME})
  list(APPEND ${PROJECT_NAME}-IMAGES ${REVISION_TARGET}-image-hex)
  list(APPEND ${PROJECT_NAME}-APPLICATIONS ${REVISION_TARGET}-bin)
  install_if_latest_revision(
    FILENAME ${HEX_IMG_FILENAME}
    COMPONENT robot-images
    THISREVISION ${REVISION}
    REVISIONS ${REVISIONS})
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}.bin
    DESTINATION applications/
    COMPONENT Applications)
  install_if_latest_revision(
    FILENAME "${REVISION_TARGET}-image.hex"
    COMPONENT images
    THISREVISION ${REVISION}
    REVISIONS ${REVISIONS})
endmacro()

foreach_revision(PROJECT_NAME rear-panel
  REVISIONS b1 b2 c1
  SOURCES rearpanel_b1_srcs rearpanel_b1_srcs rearpanel_b1_srcs
  CALL_FOREACH_REV rearpanel_loop
  NO_CREATE_IMAGE_HEX
  NO_CREATE_APPLICATION_HEX
  NO_CREATE_INSTALL_RULES
  NO_CREATE_DEBUG_TARGET
  )

add_clang_tidy_target(TARGET_NAME rear-panel-lint
                      LINT_SOURCES ${REAR_PANEL_FW_LINTABLE_SRCS})
