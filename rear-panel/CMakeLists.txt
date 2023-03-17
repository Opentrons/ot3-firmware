# rear source tree

add_subdirectory(core)

if (${CMAKE_CROSSCOMPILING})
    add_subdirectory(startup)
    add_subdirectory(firmware)
else()
   #add_subdirectory(simulator)
   add_subdirectory(test)
endif()

file(GLOB_RECURSE REAR_PANEL_SOURCES_FOR_FORMAT ./*.cpp ./*.hpp ../include/rear-panel/*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware_l5/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
  rear-panel-format
  ALL
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${REAR_PANEL_SOURCES_FOR_FORMAT}
  )

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
  rear-panel-format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${REAR_PANEL_SOURCES_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
