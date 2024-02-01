# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
include(RevisionHandling)

add_STM32G4_driver("Bootloader")

add_subdirectory("startup")


set(BOOTLOADER_G4FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/node_id_stm32g4xx.c)


set(_g4_sources ${BOOTLOADER_FW_LINTABLE_SRCS} ${BOOTLOADER_FW_NON_LINTABLE_SRCS} ${BOOTLOADER_G4FW_NON_LINTABLE_SRCS})


macro(head_bootloader_loop)
  target_link_libraries(${REVISION_TARGET}
      PUBLIC STM32G491RETx_bootloader
      STM32G4xx_Drivers_Bootloader
      bootloader-core)

  target_compile_definitions(${REVISION_TARGET} PUBLIC node_id_head)

endmacro()

set(head_sources ${_g4_sources} ${CMAKE_CURRENT_SOURCE_DIR}/head_startup.c)

foreach_revision(
    PROJECT_NAME bootloader-head
    CALL_FOREACH_REV head_bootloader_loop
    REVISIONS A1 B1 C2
    SOURCES head_sources head_sources head_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

macro(gantry_bootloader_loop)
    set(_gbl_options)
    set(_gbl_onevalue AXIS)
    set(_gbl_multivalue)
    cmake_parse_arguments(_gbl "${_gbl_options}" "${_gbl_onevalue}" "${_gbl_multivalue}" ${ARGN})
  target_link_libraries(${REVISION_TARGET}
      PUBLIC STM32G491RETx_bootloader
      STM32G4xx_Drivers_Bootloader
      bootloader-core)

  target_compile_definitions(${REVISION_TARGET} PUBLIC node_id_gantry_${_gbl_AXIS})
endmacro()

macro(gantry_x_bootloader_loop)
    gantry_bootloader_loop(AXIS x)
endmacro()

macro(gantry_y_bootloader_loop)
    gantry_bootloader_loop(AXIS y)
endmacro()

foreach_revision(
    PROJECT_NAME bootloader-gantry-x
    CALL_FOREACH_REV gantry_x_bootloader_loop
    REVISIONS a1 b1 c1 c2
    SOURCES _g4_sources _g4_sources _g4_sources _g4_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

foreach_revision(
    PROJECT_NAME bootloader-gantry-y
    CALL_FOREACH_REV gantry_y_bootloader_loop
    REVISIONS a1 b1 c1 c2
    SOURCES _g4_sources _g4_sources _g4_sources _g4_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

set(gripper_sources ${_g4_sources} ${CMAKE_CURRENT_SOURCE_DIR}/gripper_startup.c)

macro(gripper_bootloader_loop)
  target_link_libraries(${REVISION_TARGET}
      PUBLIC STM32G491RETx_bootloader
      STM32G4xx_Drivers_Bootloader
      bootloader-core)

  target_compile_definitions(${REVISION_TARGET} PUBLIC node_id_gripper)
endmacro()

foreach_revision(
    PROJECT_NAME bootloader-gripper
    CALL_FOREACH_REV gripper_bootloader_loop
    REVISIONS a1 b1 c1 c2
    SOURCES gripper_sources gripper_sources gripper_sources gripper_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

macro(pipettes_single_bootloader_loop)

  target_compile_definitions(${REVISION_TARGET} PUBLIC PIPETTE_TYPE_DEFINE=SINGLE_CHANNEL node_id_pipette_dynamic)

  target_link_libraries(${REVISION_TARGET}
    PUBLIC STM32G491RETx_bootloader
    STM32G4xx_Drivers_Bootloader
    bootloader-core)
endmacro()

macro(pipettes_multi_bootloader_loop)
  target_compile_definitions(${REVISION_TARGET} PUBLIC  PIPETTE_TYPE_DEFINE=EIGHT_CHANNEL node_id_pipette_dynamic)

  target_link_libraries(${REVISION_TARGET}
    PUBLIC STM32G491RETx_bootloader
    STM32G4xx_Drivers_Bootloader
    bootloader-core)
endmacro()

macro(pipettes_ninety_six_bootloader_loop)
  target_compile_definitions(${REVISION_TARGET} PUBLIC  PIPETTE_TYPE_DEFINE=NINETY_SIX_CHANNEL node_id_pipette_dynamic)

  target_link_libraries(${REVISION_TARGET}
    PUBLIC STM32G491RETx_bootloader
    STM32G4xx_Drivers_Bootloader
    bootloader-core)
endmacro()

set(_pipette_sources ${_g4_sources} ./pipette_handle_messages.c)

foreach_revision(
    PROJECT_NAME bootloader-pipettes-single
    CALL_FOREACH_REV pipettes_single_bootloader_loop
    REVISIONS b1 c2 d1
    SOURCES _pipette_sources _pipette_sources _pipette_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

foreach_revision(
    PROJECT_NAME bootloader-pipettes-multi
    CALL_FOREACH_REV pipettes_multi_bootloader_loop
    SOURCES _pipette_sources _pipette_sources _pipette_sources
    REVISIONS b1 c2 d1
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

foreach_revision(
    PROJECT_NAME bootloader-pipettes-96
    CALL_FOREACH_REV pipettes_ninety_six_bootloader_loop
    REVISIONS b1 c1 d2
    SOURCES _pipette_sources _pipette_sources _pipette_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

set(hepauv_sources ${_g4_sources})

macro(hepauv_bootloader_loop)
  target_link_libraries(${REVISION_TARGET}
      PUBLIC STM32G491RETx_bootloader
      STM32G4xx_Drivers_Bootloader
      bootloader-core)

  target_compile_definitions(${REVISION_TARGET} PUBLIC node_id_hepa_uv)
endmacro()

foreach_revision(
    PROJECT_NAME bootloader-hepa-uv
    CALL_FOREACH_REV hepauv_bootloader_loop
    REVISIONS a1 b1
    SOURCES hepauv_sources hepauv_sources
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES
    )

target_include_directories(STM32G4xx_Drivers_Bootloader
        PUBLIC .)

target_compile_definitions(STM32G4xx_Drivers_Bootloader
        PUBLIC STM32G491xx)
