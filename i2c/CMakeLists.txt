if (${CMAKE_CROSSCOMPILING})
  add_subdirectory(firmware)
else()
  add_subdirectory(simulation)
  add_subdirectory(tests)
endif()

find_package(Clang)

file(GLOB_RECURSE I2C_SOURCE_FOR_FORMAT ./*.cpp ./*.hpp ../include/i2c/*.hpp)

add_custom_target(
  i2c-format
  ALL
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -i ${I2C_SOURCE_FOR_FORMAT}
  )

add_custom_target(
  i2c-format-ci
  COMMAND ${Clang_CLANGFORMAT_EXECUTABLE} -style=file -Werror -ferror-limit=0 -n ${I2C_SOURCE_FOR_FORMAT}
  )

set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
