#[=======================================================================[.rst:
FindFlatbuffers.cmake
----------------

This module is intended for use with ``find_package`` and should not be imported on
its own.

#]=======================================================================]

Include(FetchContent)

FetchContent_Declare(
  flatbuffers
  GIT_REPOSITORY "https://github.com/google/flatbuffers"
  GIT_TAG        "v1.12.0"
  PREFIX         ${CMAKE_SOURCE_DIR}/stm32-tools/flatbuffers
)

FetchContent_Populate(flatbuffers)

FetchContent_GetProperties(flatbuffers
    POPULATED Flatbuffers_POPULATED
    SOURCE_DIR FlatBuffers_SOURCE_DIR
    BINARY_DIR FlatBuffers_BINARY_DIR
)

set(FlatBuffers_FOUND TRUE)

