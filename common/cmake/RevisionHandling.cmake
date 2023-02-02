#[=======================================================================[.rst:
RevisionHandling
----------------

This is a CMake module providing macros and functions to handle compiling
firmware for multiple different revisions.


Usage
+++++

To use this module, make sure you're setting the cmake module path to this
directory and call
```
include(RevisionHandling)
```

Then, you can use the macros defined in here.


Provided Macros
++++++++++++++++

install_if_latest_revision(FILENAME name
                           COMPONENT component
                           THISREVISION revision
                           REVISIONS revision1 revision2...)

Create install rules for the specified file into the specified component, if and only
if we're in a foreach_revision macro callback for the latest version. The file will be
installed into a dist subdirectory named after the component.


alias_for_revision(PROJECT_NAME name REVISION revision REVISION_ALIAS alias)

Create dummy targets for all targets of a specific revision/project pair according to
the specified names. This is used to provide a smooth transition between referring to
revisions by their overall system engineering phase (EVT, proto) and their PCBA primary
revision. Targets will be created for
- the application binary
- the application hex file
- the application binary file
- the image hex file
- flash
- debug

The PROJECT_NAME should be the same as what was passed to foreach_revision (see below).
REVISION should be the PCBA revision you want to alias. REVISION_ALIAS should be the
new name for the alias.

foreach_revision(PROJECT_NAME name
                 CALL_FOREACH_REV some_macro
                 DEFAULT_REVISION revision
                 REVISIONS revision1 revision2...
                 SOURCES sourceset1 sourceset2...
                 [NO_CREATE_EXECUTABLE]
                 [NO_CREATE_APPLICATION_HEX]
                 [NO_CREATE_IMAGE_HEX]
                 [NO_SET_COMMON_PROPERTIES]
                 [NO_PROVIDE_REVISION_DEFINES]
                 [NO_CREATE_WRAPUP_TARGETS]
                 [NO_CREATE_INSTALL_RULES]
                 [NO_EXTEND_GLOBAL_TARGETS]
                 )

These paired macros introduce a foreach() loop that moves through the provided
revisions provide various functionality
that is previously open-coded, such as:
- Create an executable target for the revision (as REVISION_TARGET, see below)
  using the specified sources, unless NO_CREATE_EXECUTABLE is specified
- Create application -hex, -bin  targets for the revision target, unless
  unless NO_CREATE_APPLICATION_HEX is specified
- Create application+bootloader -image-hex targets for the revision target, unless
  NO_CREATE_IMAGE_HEX is specified. For a bootloader to be linked in, an appropriate bootloader
  target must exist for the REVISION_TARGET (i.e. a target named bootloader-${REVISION_TARGET}-hex
  must be defined somewhere in the project - the best way to do this is to use this macro for the
  bootloader too.)
- Add the root include directory and define common target properties for C and CXX standards,
  unless NO_SET_COMMON_PROPERTIES is specified
- add a compile definition to the base target called PCBA_REVISION with the
  provided revision string; PCBA_REVISION_PRIMARY with the primary version char;
  and PCBA_REVISION_SECONDARY with the secondary version char; this will cause version.h to provide
  a way to get a struct containing the primary and secondary revision, and generate c code to fulfill
  it, unless NO_PROVIDE_REVISION_DEFINES is specified
- create wrapup targets that will build all leaf image types for all revisions unless
  NO_CREATE_WRAPUP_TARGETS is set. These are built from the ${PROJECT_NAME}-IMAGES,
  ${PROJECT_NAME}-EXES, and ${PROJECT_NAME}-APPLICATIONS variables and thus anything that removes
  content from those variables will remove content from the targets.
- create install rules unless NO_CREATE_INSTALL_RULES is set that will put hex files in (by default) dist/.
  The can be changed by altering the cache variable FIRMWARE_INSTALL_DIRECTORY. image files will go in
  dist/images; applications in  dist/applications. Images are given the component IMAGES and applications
  the component APPLICATIONS.
- add the image target as a dependency of firmware-images and the application target as a dependency of
  firmware-applications unless NO_EXTEND_GLOBAL_TARGETS is set

The start macro takes the following named arguments:

PROJECT_NAME: the name of the base thing you're compiling, e.g. head
DEFAULT_REVISION: a revision to consider the "default" and create unsuffixed targets for. If not
                  specified, no such targets will be created.
REVISIONS: the named PCBA revisions you want to support firmware for, e.g. C1, B2
SOURCES: A list of variables containing lists of source files for each revision. Mandatory unless
         NO_CREATE_EXECUTABLE is set.
ARCHITECTURES: A list of architectures in (STM32G4, STM32L5), one for each specified revision.
               If not specified, defaults to G4.
CALL_FOREACH_REV: A macro (it really needs to be a macro because we don't want a new scope here)
                  that will be executed once for each revision.
NO_CREATE_EXECUTABLE: If provided, do not create executable targets. If you provide this,
                      but don't provide the other NO_* arguments, make sure you've created
                      executables with ``add_executable`` that have names like
                      ${PROJECT_NAME}-${REVISION} for each revision before you call the macro.
NO_CREATE_APPLICATION_HEX: If provided, do not create the custom targets and commands to make
                           hex files from the applications. If you specify this but not
                           NO_CREATE_IMAGE_HEX, make sure you create a target called
                           ${PROJECT_NAME}-${REVISION}-${HEX}.
NO_CREATE_DEBUG_TARGET: If provided, do not create the -debug target.
NO_CREATE_IMAGE_HEX: If provided, do not create image hexes that fuse the bootloader and
                     application.
NO_SET_COMMON_PROPERTIES: If provided, do not set common c and C++ language properties and
                          include directories
NO_PROVIDE_REVISION_DEFINES: If provided, do not provide C/C++ preprocessor defines for the PCBA
                             revisions or compiled-in extern variables containing same.
NO_CREATE_INSTALL_RULES: If provided, do not create rules that will install image and application
                         hex to dist/.
NO_EXTEND_GLOBAL_TARGETS: If provided, do not add the wrapup targets to the global wrapup targets.

Inside the macro, the following variables are defined:
- PROJECT_NAME: The project name passed in to the macro
- REVISION: The revision currently being created
- REVISION_TARGET: the name of the executable
- REVISION_HEX_TARGET: the name of the application firmware hex
- REVISION_BIN_TARGET: the name of the application firmware bin file
- REVISION_HEX_IMAGE_TARGET: the name of the application+bootloader firmware hex image
- REVISION_DEBUG_TARGET: the name of the debug target for the revision
- REVISION_FLASH_TARGET: the name of the flash target for the revision

The macro will define the following targets (if ${REVISION} is in there, it's for each revision)
unless the relevant NO_* option is defined:
${PROJECT_NAME}-${REVISION} - the application firmware binary
${PROJECT_NAME}-${REVISION}-hex - a custom_target of the application firmware hex file
${PROJECT_NAME}-${REVISION}-binary - a custom_target of the application firmware bin file
${PROJECT_NAME}-${REVISION}-image-hex - a custom_target packing in the bootloader in a hex file
${PROJECT_NAME}-${REVISION}-flash - a custom_target that will flash the image hex using openocd
${PROJECT_NAME}-${REVISION}-debug - a custom_target that will run gdb via openocd with the application firmware

The macro will end with the following variables defined (in addition to those used inside the
macro, which will have their value as of the last loop of the macro):

${PROJECT_NAME}-EXES: A list of the executables created during the macro (empty if NO_CREATE_EXECUTABLE is set)
${PROJECT_NAME}-APPLICATIONS: A list of the application hexes created during the macro (empty if
                              NO_CREATE_APPLICATION_HEX is set)
${PROJECT_NAME}-IMAGES: A list of the image hexes created during the macro (empty if NO_CREATE_IMAGE_HEX is set)

#]=======================================================================]

macro(foreach_revision)
set(_fer_options
    NO_CREATE_EXECUTABLE
    NO_CREATE_APPLICATION_HEX
    NO_CREATE_IMAGE_HEX
    NO_CREATE_DEBUG_TARGET
    NO_SET_COMMON_PROPERTIES
    NO_PROVIDE_REVISION_DEFINES
    NO_CREATE_INSTALL_RULES)
set(_fer_onevalue PROJECT_NAME DEFAULT_REVISION CALL_FOREACH_REV)
set(_fer_multivalue REVISIONS SOURCES ARCHITECTURES)
cmake_parse_arguments(_fer "${_fer_options}" "${_fer_onevalue}" "${_fer_multivalue}" ${ARGN})
list(LENGTH _fer_SOURCES _fer_sourcelength)
list(LENGTH _fer_REVISIONS _fer_revisionlength)
set(PROJECT_NAME ${_fer_PROJECT_NAME})
set(REVISIONS ${_fer_REVISIONS})
if (NOT _fer_sourcelength EQUAL _fer_revisionlength)
  if (NOT _fer_NO_CREATE_EXECUTABLE)
  message(FATAL "There must be a 1:1 mapping of sources and revisions, got sources: ${_fer_SOURCES} and revisions ${_fer_REVISIONS}")
  endif()
endif()
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


# we'll be modifying the source list as we iterate through the revision list so make a copy to
# alter
set(_fer_revision_sources ${_fer_SOURCES})
set(_fer_revision_architectures ${_fer_ARCHITECTURES})
set(${PROJECT_NAME}-EXES)
set(${PROJECT_NAME}-APPLICATIONS)
set(${PROJECT_NAME}-IMAGES)
message(STATUS "Creating targets for ${PROJECT_NAME} with revisions ${REVISIONS}")
list(APPEND CMAKE_MESSAGE_INDENT " (${PROJECT_NAME}) ")

foreach(_REVISION_MIXEDCASE IN LISTS _fer_REVISIONS)
  string(TOLOWER ${_REVISION_MIXEDCASE} REVISION)
  set(REVISION_TARGET "${PROJECT_NAME}-${REVISION}")
  message(VERBOSE "Considering ${PROJECT_NAME}-${REVISION}")

  set(REVISION_DEBUG_TARGET ${REVISION_TARGET}-debug)
  list(GET _fer_revision_architectures 0 _fer_thisrevision_arch)
  list(POP_FRONT _fer_revision_architectures)
  if (NOT _fer_thisrevision_arch)
    set(_fer_thisrevision_arch "STM32G4")
    message(STATUS "Defaulted ${REVISION_TARGET} to architecture ${_fer_thisrevision_arch}")
  endif()
  list(GET _fer_revision_sources 0 _fer_thisrevision_sources)
  list(POP_FRONT _fer_revision_sources)

  if (NOT _fer_NO_CREATE_EXECUTABLE)
    message(STATUS "Added executable ${REVISION_TARGET}")
    add_executable(${REVISION_TARGET} ${${_fer_thisrevision_sources}})
    list(APPEND ${PROJECT_NAME}-EXES ${REVISION_TARGET})
  else()
    message(VERBOSE "Not adding executable (inhibited by NO_CREATE_EXECUTABLE)")
  endif()

  if (NOT _fer_NO_SET_REVISION_DEFINES)
    string(SUBSTRING ${REVISION} 0 1 PRIMARY_REVISION)
    string(SUBSTRING ${REVISION} 1 1 SECONDARY_REVISION)
    target_compile_definitions(${REVISION_TARGET} PUBLIC
      PCBA_REVISION=${REVISION}
      PCBA_PRIMARY_REVISION=${REVISION_PRIMARY}
      PCBA_SECONDARY_REVISION=${REVISION_SECONDARY})
    configure_file(
      ${CMAKE_SOURCE_DIR}/common/core/revision.c.in
      ${CMAKE_BINARY_DIR}/common/core/revision-${REVISION_TARGET}.c)
    target_sources(${REVISION_TARGET} PRIVATE
      ${CMAKE_BINARY_DIR}/common/core/revision-${REVISION_TARGET}.c)
    message(VERBOSE "Added revision defines primary ${PRIMARY_REVISION} secondary ${SECONDARY_REVISION}")
  else()
    message(VERBOSE "Not adding revision defines (inhibited by NO_SET_REVISION_DEFINES)")
  endif()
  set(REVISION_HEX_TARGET ${REVISION_TARGET}-hex)
  set(REVISION_HEX_FILE ${REVISION_TARGET}.hex)
  set(REVISION_BIN_TARGET ${REVISION_TARGET}-bin)
  set(REVISION_BIN_FILE ${REVISION_TARGET}.bin)
  if (NOT _fer_NO_CREATE_APPLICATION_HEX)

    add_custom_command(OUTPUT ${REVISION_HEX_FILE}
          COMMAND ${CROSS_OBJCOPY} ARGS ${REVISION_TARGET} "-Oihex" ${REVISION_HEX_FILE}
          DEPENDS ${REVISION_TARGET}
          VERBATIM)

    add_custom_target(${REVISION_HEX_TARGET} ALL
         DEPENDS ${REVISION_HEX_FILE})
    list(APPEND IMAGES ${REVISION_HEX_TARGET})

    add_custom_command(OUTPUT ${REVISION_BIN_FILE}
          COMMAND ${CROSS_OBJCOPY} ARGS ${REVISION_TARGET} "-Obinary" ${REVISION_BIN_FILE}
          DEPENDS ${REVISION_TARGET}
          VERBATIM)
    add_custom_target(${REVISION_BIN_TARGET} ALL
          DEPENDS ${REVISION_BIN_FILE})
    list(APPEND ${PROJECT_NAME}-APPLICATIONS ${REVISION_HEX_TARGET})
    message(STATUS "Added application .hex ${REVISION_HEX_FILE} as ${REVISION_HEX_TARGET}")
    message(STATUS "Added application .bin ${REVISION_BIN_FILE} as ${REVISION_BIN_TARGET}")
  else()
    message(VERBOSE "Not creating application .hex (inhibited by NO_CREATE_APPLICATION_HEX)")
    message(VERBOSE "Not creating application .bin (inhibited by NO_CREATE_APPLICATION_HEX)")
  endif()
  if (NOT _fer_NO_CREATE_DEBUG_TARGET)
      if (_fer_thisrevision_arch STREQUAL "STM32G4")
        set(_fer_gdbinit ${CMAKE_BINARY_DIR}/common/firmware/STM32G491RETx/gdbinit)
      elseif(_fer_thisrevision_arch STREQUAL "STM32L5")
        set(_fer_gdbinit ${CMAKE_BINARY_DIR}/common/firmware/STM32L562RETx/gdbinit)
      else()
        message(FATAL_ERROR "Unknown architecture ${_fer_thisrevision_arch} for ${TARGET_REVISION}")
      endif()
      # Configure gdb (full path to cross-gdb set in the toolchain) to use the gdbinit in
      # the common directory
      set_target_properties(${REVISION_TARGET}
          PROPERTIES
          CROSSCOMPILING_EMULATOR
          "${ARM_GDB};--command=${_fer_gdbinit}")
      # Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
      # arguable misuse of the concept) to the appropriate cross-gdb with
      # remote-target. You should make sure st-util is running; that's not
      # done here because it won't be multi-os compatible, and also it
      # should be running the entire time and that's tough to accomplish
      # in a custom command
      add_custom_target(${REVISION_DEBUG_TARGET}
          COMMAND ${REVISION_TARGET}
          USES_TERMINAL
       )
     message(STATUS "Added debug target ${REVISION_DEBUG_TARGET}")
     message(VERBOSE "Using gdbinit ${_fer_gdbinit}")
  else()
     message(VERBOSE "Not adding debug target (inhibited by NO_CREATE_DEBUG_TARGET)")
  endif()
  set(REVISION_FLASH_TARGET ${REVISION_TARGET}-flash)
  # Targets to create full image hex file containing both bootloader and application
  set(REVISION_HEX_IMAGE_FILE ${REVISION_TARGET}-image.hex)
  set(REVISION_HEX_IMAGE_TARGET ${REVISION_TARGET}-image-hex)

  if (NOT _fer_NO_CREATE_IMAGE_HEX)
      add_custom_command(
          OUTPUT ${REVISION_HEX_IMAGE_FILE}
          DEPENDS ${REVISION_HEX_TARGET} bootloader-${REVISION_TARGET}-hex ${REVISION_HEX_FILE} $<TARGET_FILE_DIR:bootloader-${REVISION_TARGET}>/bootloader-${REVISION_TARGET}.hex
          COMMAND ${CMAKE_SOURCE_DIR}/scripts/hex_combine.py ${REVISION_HEX_IMAGE_FILE} $<TARGET_FILE_DIR:bootloader-${REVISION_TARGET}>/bootloader-${REVISION_TARGET}.hex ${REVISION_HEX_FILE}
          VERBATIM)
      add_custom_target(${REVISION_HEX_IMAGE_TARGET} ALL
          DEPENDS ${REVISION_HEX_IMAGE_FILE})
      if (_fer_thisrevision_arch STREQUAL "STM32G4")
        set(_fer_discofile "${CMAKE_SOURCE_DIR}/common/firmware/STM32G491RETx/stm32g4discovery.cfg")
      elseif(_fer_thisrevision_arch STREQUAL "STM32L5")
         set(_fer_discofile "${CMAKE_SOURCE_DIR}/common/firmware/STM32L562RETx/stm32l5discovery.cfg")
      else()
        message(FATAL_ERROR "Unknown architecture ${_fer_thisrevision_arch} for ${TARGET_REVISION}")
      endif()

      # Targets to flash bootloader and firmware
      add_custom_target(${REVISION_FLASH_TARGET}
          COMMAND "${OpenOCD_EXECUTABLE}" "-f" "${_fer_discofile}" "-c" "program ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_TARGET}-image.hex;reset;exit"
          VERBATIM
          COMMENT "Flashing board"
          DEPENDS ${REVISION_HEX_IMAGE_TARGET})
      list(APPEND ${PROJECT_NAME}-IMAGES ${REVISION_HEX_IMAGE_TARGET})
      message(STATUS "Added image .hex ${REVISION_HEX_IMAGE_FILE} as ${REVISION_HEX_IMAGE_TARGET}")
      message(STATUS "Added flash target ${REVISION_FLASH_TARGET}")
      message(VERBOSE "Using flash script ${_fer_discofile}")
  else()
    message(VERBOSE "Not adding image .hex (inhibited by NO_CREATE_IMAGE_HEX)")
    message(VERBOSE "Not adding flash target (inhibited by NO_CREATE_IMAGE_HEX)")
  endif()
  if (NOT _fer_NO_SET_COMMON_PROPERTIES)
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
      message(VERBOSE "Added common properties")
  elseif()
      message(VERBOSE "Not adding common properties (inhibited by NO_SET_COMMON_PROPERTIES)")
  endif()
  if (_fer_CALL_FOREACH_REV)
      message(VERBOSE "Calling user-provided callback")
      cmake_language(CALL ${_fer_CALL_FOREACH_REV})
      message(VERBOSE "Called user-provided callback")
  else()
      message(VERBOSE "Not calling user-provided callback (not passed in CALL_FOREACH_REV) ")
  endif()
  if (NOT _fer_NO_CREATE_INSTALL_RULES)
    if (NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
      install(
          FILES ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_HEX_FILE}
          DESTINATION applications/
          COMPONENT Applications)
      install(
          FILES ${CMAKE_CURRENT_BINARY_DIR}/${REVISION_HEX_IMAGE_FILE}
          DESTINATION images/
          COMPONENT Images)
      message(STATUS "Creating install rule for ${HEX_FILE}")
    else()
      message(WARNING "CMAKE_INSTALL_PREFIX is not set, not creating install rules")
    endif()
  else()
    message(VERBOSE "Not creating install rules (inhibited by NO_CREATE_INSTALL_RULES)")
  endif()
endforeach()
if (_fer_DEFAULT_REVISION)
    if (NOT _fer_NO_CREATE_EXECUTABLE)
        add_custom_target(${PROJECT_NAME}
          ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}-${_fer_DEFAULT_REVISION} ${PROJECT_NAME}
          DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION})
        message(STATUS "Created default revision executable ${PROJECT_NAME}")
    else()
        message(VERBOSE "Not creating default revision executable (inhibited by NO_CREATE_EXECUTABLE)")
    endif()
    if (NOT _fer_NO_CREATE_IMAGE_HEX)
        add_custom_command(OUTPUT ${PROJECT_NAME}-image.hex
          COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-image.hex ${PROJECT_NAME}-image.hex
          DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-image-hex
        )
        add_custom_target(${PROJECT_NAME}-image-hex DEPENDS ${PROJECT_NAME}_image.hex)
        add_custom_target(${PROJECT_NAME}-flash DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-flash)
        message(STATUS "Created default revision image .hex ${PROJECT_NAME}-image.hex as ${PROJECT_NAME}-image-hex")
        message(STATUS "Created default revision flash target ${PROJECT_NAME}-flash")
    else()
        message(VERBOSE "Not creating default revision image .hex (inhibited by NO_CREATE_IMAGE_HEX)")
        message(VERBOSE "Not creating default revision flash target (inhibited by NO_CREATE_IMAGE_HEX)")
    endif()
    add_custom_command(OUTPUT ${PROJECT_NAME}.hex
      COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}.hex ${PROJECT_NAME}.hex
      DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-hex
    )
    add_custom_target(${PROJECT_NAME}-hex DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-hex)
    add_custom_target(${PROJECT_NAME}-debug DEPENDS ${PROJECT_NAME}-${_fer_DEFAULT_REVISION}-debug)
    message(STATUS "Created default revision application .hex ${PROJECT_NAME}.hex as ${PROJECT_NAME}-hex")
    message(STATUS "Created default revision debug target as ${PROJET_NAME}-debug")
else()
    message(VERBOSE "Not creating default revision executable (no DEFAULT_REVISION passed)")
    message(VERBOSE "Not creating default revision image .hex (no DEFAULT_REVISION passed)")
    message(VERBOSE "Not creating default revision flash target (no DEFAULT_REVISION passed)")
    message(VERBOSE "Not creating default revision application .hex (no DEFAULT_REVISION passed)")
    message(VERBOSE "Not creating default revision debug target (no DEFAULT_REVISION passed)")
endif()
if (NOT _fer_NO_CREATE_WRAPUP_TARGETS)
    add_custom_target(${PROJECT_NAME}-exes DEPENDS ${${PROJECT_NAME}-EXES})
    add_custom_target(${PROJECT_NAME}-images DEPENDS ${${PROJECT_NAME}-IMAGES})
    add_custom_target(${PROJECT_NAME}-applications DEPENDS ${${PROJECT_NAME}-APPLICATIONS})
    message(STATUS "Created summary target ${PROJECT_NAME}-exes to build ${${PROJECT_NAME}-EXES}")
    message(STATUS "Created summary target ${PROJECT_NAME}-images to build ${${PROJECT_NAME}-IMAGES}")
    message(STATUS "Created summary target ${PROJECT_NAME}-applications to build ${${PROJECT_NAME}-APPLICATIONS}")
    if (NOT _fer_NO_EXTEND_GLOBAL_TARGET)
        add_dependencies(firmware-images ${PROJECT_NAME}-images)
        add_dependencies(firmware-applications ${PROJECT_NAME}-applications)
        message(STATUS "Added ${PROJECT_NAME}-images to firmware-images")
        message(STATUS "Added ${PROJECT_NAME}-applications to application-images")
    else()
        message(VERBOSE "Not adding ${PROJECT_NAME}-images to firmware-images (inhibited by NO_EXTEND_GLOBAL_TARGET)")
        message(VERBOSE "Not adding ${PROJECT_NAME}-applications to firmware-applications (inhibited by NO_EXTEND_GLOBAL_TARGET)")
    endif()
else()
    message(VERBOSE "Not creating summary target ${PROJECT_NAME}-exes (inhibited by NO_CREATE_WRAPUP_TARGETS)")
    message(VERBOSE "Not creating summary target ${PROJECT_NAME}-images (inhibited by NO_CREATE_WRAPUP_TARGETS)")
    message(VERBOSE "Not creating summary target ${PROJECT_NAME}-applications (inhibited by NO_CREATE_WRAPUP_TARGETS)")
    message(VERBOSE "Not adding ${PROJECT_NAME}-images to firmware-images (inhibited by NO_CREATE_WRAPUP_TARGETS)")
    message(VERBOSE "Not adding ${PROJECT_NAME}-applications to firmware-applications (inhibited by NO_CREATE_WRAPUP_TARGETS)")
endif()
list(POP_BACK CMAKE_MESSAGE_INDENT)
endmacro()

macro(alias_for_revision)
  set(_afr_options)
  set(_afr_onevalue PROJECT_NAME REVISION REVISION_ALIAS)
  set(_afr_multivalue )
  cmake_parse_arguments(_afr "${_afr_options}" "${_afr_onevalue}" "${_afr_multivalue}" ${ARGN})
  string(TOLOWER ${_afr_REVISION} _base_revision)
  string(TOLOWER ${_afr_REVISION_ALIAS} _revision_alias)
  add_custom_target(${_afr_PROJECT_NAME}-${_revision_alias} DEPENDS ${_afr_PROJECT_NAME}-${_base_revision})
  add_custom_target(${_afr_PROJECT_NAME}-${_revision_alias}-hex DEPENDS ${_afr_PROJECT_NAME}-${_base_revision})
  add_custom_target(${_afr_PROJECT_NAME}-${_revision_alias}-image-hex DEPENDS ${_afr_PROJECT_NAME}-${_base_revision}-image-hex)
  add_custom_target(${_afr_PROJECT_NAME}-${_revision_alias}-flash DEPENDS ${_afr_PROJECT_NAME}-${_base_revision}-flash)
  add_custom_target(${_afr_PROJECT_NAME}-${_revision_alias}-debug DEPENDS ${_afr_PROJECT_NAME}-${_base_revision}-debug)
  message(STATUS "Created alias ${_afr_PROJECT_NAME}-${_revision_alias} for ${_afr_PROJECT_NAME}-${_base_revision} exe, hex, flash, debug")
endmacro()

macro(install_if_latest_revision)
  set(_ilr_options)
  set(_ilr_onevalue FILENAME COMPONENT THISREVISION)
  set(_ilr_multivalue REVISIONS)
  cmake_parse_arguments(_ilr "${_irl_options}" "${_ilr_onevalue}" "${_ilr_multivalue}" ${ARGN})
  list(GET _ilr_REVISIONS -1 _ilr_recent_rev)
  if (_ilr_THISREVISION STREQUAL _ilr_recent_rev)
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/${_ilr_FILENAME}
      DESTINATION ${_ilr_COMPONENT}
      COMPONENT ${_ilr_COMPONENT}
      )
  endif()
endmacro()
