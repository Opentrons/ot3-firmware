# CMake 3.19 is the first to include presets
# (https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html)
# cmake_minimum_required(VERSION 3.19)

add_custom_target(
  format-flat
  ALL
  COMMAND ${FlatBuffers_BINARY_DIR}/flatc --cpp -o ${CMAKE_CURRENT_SOURCE_DIR}/../include/messages ${CMAKE_CURRENT_SOURCE_DIR}/messages.fbs
)
