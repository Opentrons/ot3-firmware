# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32L5xx)

add_STM32L5_driver("Bootloader")

add_subdirectory("startup")

target_include_directories(STM32L5xx_Drivers_Bootloader
        PUBLIC .)

target_compile_definitions(STM32L5xx_Drivers_Bootloader
        PUBLIC STM32L562xx)

set(BOOTLOADER_L5_FW_NON_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32l5xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/stm32l5xx_it.c
        ${CMAKE_CURRENT_SOURCE_DIR}/clocking.c
        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
        ${CMAKE_CURRENT_SOURCE_DIR}/node_id_stm32l5xx.c)

macro(pipettes_bootloader_loop)
    target_link_libraries(${REVISION_TARGET}
            PUBLIC STM32L562RETx_bootloader
            STM32L5xx_Drivers_Bootloader
            bootloader-core)
    target_compile_definitions(${REVISION_TARGET} PUBLIC node_id_pipette_dynamic)
endmacro()

set(srcs ${BOOTLOADER_FW_LINTABLE_SRCS}
         ${BOOTLOADER_FW_NON_LINTABLE_SRCS}
         ${BOOTLOADER_L5_FW_NON_LINTABLE_SRCS})

foreach_revision(
    PROJECT_NAME bootloader-pipettes-single
    REVISIONS a1
    SOURCES srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_bootloader_loop
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES)

foreach_revision(
    PROJECT_NAME bootloader-pipettes-multi
    REVISIONS a1
    SOURCES srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_bootloader_loop
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES)

foreach_revision(
    PROJECT_NAME bootloader-pipettes-96
    REVISIONS a1
    SOURCES srcs
    ARCHITECTURES STM32L5
    CALL_FOREACH_REV pipettes_bootloader_loop
    NO_CREATE_IMAGE_HEX
    NO_CREATE_INSTALL_RULES)
