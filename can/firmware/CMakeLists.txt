# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.
find_package(STM32G4xx)
#add_STM32G4_driver("Can")
#add_STM32G4_freertos("Can")

# Add source files that should be checked by clang-tidy here
set(CAN_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/hal_can_bus.cpp)

#set(CAN_FW_NON_LINTABLE_SRCS
#        ${CMAKE_CURRENT_SOURCE_DIR}/system_stm32g4xx.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/stm32g4xx_it.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/uart.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/spi.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/can.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/motor.c)

add_library(can STATIC
        ${CAN_FW_LINTABLE_SRCS})
#        ${CAN_FW_NON_LINTABLE_SRCS})


set_target_properties(can
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)

target_include_directories(
        can
        PUBLIC ${CMAKE_SOURCE_DIR}/include
        ${bsp_source}/Drivers/STM32G4xx_HAL_Driver/Inc
        ${bsp_source}/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy
        ${bsp_source}/Drivers/CMSIS/Device/ST/STM32G4xx/Include
        ${bsp_source}/Drivers/CMSIS/Core/Include)


target_compile_options(can
        PUBLIC
        -Wall
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
#
#target_include_directories(STM32G4xx_Drivers_Can
#        PUBLIC .)
#target_include_directories(STM32G4xx_FreeRTOS_Can
#        PUBLIC .)
#
target_compile_definitions(can
        PUBLIC STM32G491xx)
#target_compile_definitions(STM32G4xx_FreeRTOS_Can
#        PUBLIC STM32G491xx)
#
#set_target_properties(
#        STM32G4xx_FreeRTOS_Can
#        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")


find_package(Clang)

# # runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# # which is a catch-all static analyzer/linter
# # the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# # An odd thing about this target is that it requires the existance of a compiledb, which
# # is produced when you build, and may change if you change compilation options, so in a way
# # it depends on a build. But we also want to be able to run this when there wasn't a successful
# # build, so there's no explicit dependency set.
# # This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# # own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# # include directories. So we can use the actually fairly cool transform command to turn them all into
# # extra-arg invocations and it'll figure it out.
set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# This helps with clang accepting what GCC accepts around the implementations of the message queue
list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(can-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${CAN_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES} --config=)
