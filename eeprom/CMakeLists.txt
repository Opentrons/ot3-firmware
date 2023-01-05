if (NOT ${CMAKE_CROSSCOMPILING})
    add_subdirectory(tests)
endif()

file(GLOB_RECURSE EEPROM_SOURCE_FOR_FORMAT ./*.cpp ./*.hpp ../include/eeprom/*.hpp)

add_custom_target(
        eeprom-format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${EEPROM_SOURCE_FOR_FORMAT}
)

add_custom_target(
        eeprom-format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror -ferror-limit=0 -n ${EEPROM_SOURCE_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
