# hepa-uv source tree

add_subdirectory(core)

if (${CMAKE_CROSSCOMPILING})
    if (${ARM_ARCH_TYPE} STREQUAL "cortex-m4")
        add_subdirectory(firmware)
    endif ()
else ()
    add_subdirectory(simulator)
    add_subdirectory(tests)
endif ()

file(GLOB_RECURSE HEPA_SOURCES_FOR_FORMAT ./*.cpp ./*.hpp ../include/hepa-uv/*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
        hepa-uv-format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${HEPA_SOURCES_FOR_FORMAT}
)

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
        hepa-uv-format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${HEPA_SOURCES_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
