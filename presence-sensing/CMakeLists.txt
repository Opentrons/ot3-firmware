add_subdirectory(core)

add_library(presence-sensing-core INTERFACE)
target_include_directories(presence-sensing-core INTERFACE ${CMAKE_CURRENT_LIST_DIR}/../include)

target_link_libraries(presence--core INTERFACE can-core)

if (${CMAKE_CROSSCOMPILING})
    add_subdirectory(firmware)
    function(target_presence_sensing TARGET)
      target_presence_sensing_firmware(${TARGET})
      target_link_libraries(${TARGET} PUBLIC motor-utils motor-control-core)
    endfunction()
else()
    function(target_presence_sensing TARGET)
      target_link_libraries(${TARGET} PUBLIC motor-utils motor-control-core)
    endfunction()
    add_subdirectory(tests)
    if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "Linux")
        # Simulator requires linux only kernel interfaces
        # add_subdirectory(simulator)
    endif()
endif()


find_package(Clang)

file(GLOB_RECURSE PRESENCESENSING_SOURCES_FOR_FORMAT ./*.cpp ./*.hpp ../include/motor-control/*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
        presence-sensing-format
        ALL
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${PRESENCESENSING_SOURCES_FOR_FORMAT}
)

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
        presence-sensing-format-ci
        COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${PRESENCESENSING_SOURCES_FOR_FORMAT}
)

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)