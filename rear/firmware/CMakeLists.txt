# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
add_STM32G4_driver("Rear")
add_STM32G4_freertos("Rear")

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

# List of revisions of the rear that we support
set(REVISIONS rear-rev1)
# The default revision that you'll get if you use targets not qualified
# by revision (i.e. if you do cmake --build --preset=rear --target rear
# rather than cmake --build --preset=rear --target rear-proto)
set(DEFAULT_REVISION rear-rev1)

# Add source files that should be checked by clang-tidy here
set(REAR_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/freertos_idle_timer_task.cpp
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.cpp
        )

set(REAR_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utility_gpio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/i2c_setup.c
        ${CMAKE_CURRENT_SOURCE_DIR}/led_hardware.c
        ${COMMON_EXECUTABLE_DIR}/system/app_update.c
        ${COMMON_EXECUTABLE_DIR}/system/iwdg.c
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        )


target_include_directories(STM32G4xx_Drivers_Rear
        PUBLIC .)
target_include_directories(STM32G4xx_FreeRTOS_Rear
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Rear
        PUBLIC STM32G491xx)
target_compile_definitions(STM32G4xx_FreeRTOS_Rear
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Rear
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")

find_program(CROSS_OBJCOPY "${CrossGCC_TRIPLE}-objcopy"
  PATHS "${CrossGCC_BINDIR}"
  NO_DEFAULT_PATH
  REQUIRED)


foreach(REVISION_TARGET IN LISTS REVISIONS )
  add_executable(${REVISION_TARGET}
          ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
          ${REAR_FW_LINTABLE_SRCS}
          ${REAR_FW_NON_LINTABLE_SRCS})
  # Incurs at least a relink when you change the linker file (and a recompile of main
  # but hopefully that's quick)
  set_source_files_properties(main.cpp
    PROPERTIES
    OBJECT_DEPENDS "${COMMON_MCU_DIR}/STM32G491RETx_FLASH.ld")
  target_rear_core_rev1(${REVISION_TARGET})
  target_i2c_firmware(${REVISION_TARGET})
  
  target_link_libraries(${REVISION_TARGET}
          PUBLIC 
          STM32G4xx_Drivers_Rear
          STM32G4xx_FreeRTOS_Rear
          rear-startup-STM32G491
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
    set(GDBINIT_FILE "${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit")
    set_target_properties(${REVISION_TARGET}
            PROPERTIES
            CROSSCOMPILING_EMULATOR
            "${ARM_GDB};--command=${GDBINIT_FILE}")
    
    # Calculate and package the integrity info for this module
  
  add_custom_target(${REVISION_TARGET}-integrity ALL 
    COMMAND ${CROSS_OBJCOPY} ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET} -Obinary 
        ${REVISION_TARGET}-scratch.bin --gap-fill=0xFF
    COMMAND ${CMAKE_SOURCE_DIR}/calculate_checksum.py 
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
  
  # Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
  # arguable misuse of the concept) to the appropriate cross-gdb with
  # remote-target. You should make sure st-util is running; that's not
  # done here because it won't be multi-os compatible, and also it
  # should be running the entire time and that's tough to accomplish
  # in a custom command
  add_custom_target(${REVISION_TARGET}-debug
    COMMENT "Starting gdb and openocd"
    COMMAND ${REVISION_TARGET}
    DEPENDS ${REVISION_TARGET}-integrity
    USES_TERMINAL
    )

  set(HEX_IMG_NAME "${CMAKE_CURRENT_BINARY_DIR}/rear@${REVISION_TARGET}.hex")

  set(BIN_IMG_NAME "${CMAKE_CURRENT_BINARY_DIR}/rear@${REVISION_TARGET}.bin")
    
  set(STARTUP_NAME ${REVISION_TARGET}-startup)

  stm32g491_clear_target(${REVISION_TARGET}-clear)

  stm32g491_startup(${STARTUP_NAME} ${REVISION_TARGET})

  # Targets to create full image hex file containing both bootloader and application
  add_custom_command(
      OUTPUT ${HEX_IMG_NAME}
      DEPENDS ${REVISION_TARGET}-hex
      DEPENDS ${STARTUP_NAME}-hex
      DEPENDS ${REVISION_TARGET}.hex
      COMMAND ${CMAKE_SOURCE_DIR}/hex_combine.py ${HEX_IMG_NAME} $<TARGET_FILE_DIR:${STARTUP_NAME}>/${STARTUP_NAME}.hex ${REVISION_TARGET}.hex
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
      COMMAND "${OpenOCD_EXECUTABLE}" "-d"
          "-f" "${COMMON_MCU_DIR}/stm32g4discovery.cfg" 
          "-c" "program ${HEX_IMG_NAME} reset;exit"
      VERBATIM
      COMMENT "Flashing board"
      DEPENDS ${HEX_IMG_NAME})
endforeach()

# runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# which is a catch-all static analyzer/linter
# the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# An odd thing about this target is that it requires the existance of a compiledb, which
# is produced when you build, and may change if you change compilation options, so in a way
# it depends on a build. But we also want to be able to run this when there wasn't a successful
# build, so there's no explicit dependency set.
# This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# include directories. So we can use the actually fairly cool transform command to turn them all into
# extra-arg invocations and it'll figure it out.
set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# This helps with clang accepting what GCC accepts around the implementations of the message queue
list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(rear-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} "--extra-arg=-DOPENTRONS_CLANG_TIDY_WORKAROUND_44178" ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${REAR_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
list(APPEND LINT_TARGETS rear-lint)
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
