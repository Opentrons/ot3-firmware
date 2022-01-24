# bootloader source tree

add_subdirectory(core)

if (${CMAKE_CROSSCOMPILING})
    add_subdirectory(firmware)
else ()
    add_subdirectory(tests)
endif ()

file(GLOB_RECURSE BOOTLOADER_SOURCES_FOR_FORMAT ./*.cpp ./*.hpp ../include/bootloader/*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
        bootloader-format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${BOOTLOADER_SOURCES_FOR_FORMAT}
)

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
        bootloader-format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${BOOTLOADER_SOURCES_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
