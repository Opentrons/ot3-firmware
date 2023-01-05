if (${CMAKE_CROSSCOMPILING})
    add_subdirectory(firmware)
else()
    add_subdirectory(simulation)
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE SPI_SOURCE_FOR_FORMAT ./*.cpp ./*.hpp ../include/spi/*.hpp)

add_custom_target(
        spi-format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${SPI_SOURCE_FOR_FORMAT}
)

add_custom_target(
        spi-format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror -ferror-limit=0 -n ${SPI_SOURCE_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)