if (NOT ${CMAKE_CROSSCOMPILING})
  add_subdirectory(tests)
endif()

find_package(Clang)

file(GLOB_RECURSE SENSORS_SOURCE_FOR_FORMAT ./*.cpp ./*.hpp ../include/sensors/*.hpp)

add_custom_target(
  sensors-format
  ALL
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${SENSORS_SOURCE_FOR_FORMAT}
  )

add_custom_target(
  sensors-format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror -ferror-limit=0 -n ${SENSORS_SOURCE_FOR_FORMAT}
  )

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
