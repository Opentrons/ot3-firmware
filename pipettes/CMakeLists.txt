# pipettes source tree
find_package(STM32G4xx)
add_STM32G4_freertos("Pipettes")

target_include_directories(STM32G4xx_FreeRTOS_Pipettes
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(STM32G4xx_FreeRTOS_Pipettes
        PUBLIC STM32G491xx)

set_target_properties(
        STM32G4xx_FreeRTOS_Pipettes
        PROPERTIES FREERTOS_HEAP_IMPLEMENTATION "heap_5")


include_directories(BEFORE PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_subdirectory(core)

if (${CMAKE_CROSSCOMPILING})
  add_subdirectory(firmware)
else()
  add_subdirectory(tests)
endif()


file(GLOB_RECURSE PIPETTES_SOURCES_FOR_FORMAT ./*.cpp ./*.hpp ../include/pipettes/*.hpp)

# Targets for formatting. These are here rather than in individual target CMakeLists (e.g.
# the ones in tests/ or firmware/) because they don't have semantic reasoning involved and
# can therefore operate on file globs, unlike lint/static analysis

# Target for use during dev - edits files
add_custom_target(
  pipettes-format
  ALL
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${PIPETTES_SOURCES_FOR_FORMAT}
  )

# Target for use in ci - warnings are errors, doesn't edit files
add_custom_target(
  pipettes-format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror --ferror-limit=0 -n ${PIPETTES_SOURCES_FOR_FORMAT}
)
