if (${ARM_ARCH_TYPE} STREQUAL "cortex-m33")
    add_subdirectory("STM32L562RETx")
else()
    add_subdirectory("STM32G491RETx")
endif()

add_subdirectory("uart")
add_subdirectory("spi")
add_subdirectory("motor_driver")
add_subdirectory("can")
add_subdirectory("i2c")

