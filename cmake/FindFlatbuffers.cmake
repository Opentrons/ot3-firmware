#[=======================================================================[.rst:
FindFlatbuffers.cmake
----------------

This module is intended for use with ``find_package`` and should not be imported on
its own.

#]=======================================================================]

Include(FetchContent)


FetchContent_Declare(
  Flatbuffers
  GIT_REPOSITORY "https://github.com/google/flatbuffers.git"
  GIT_TAG        "v1.12.1"
  PREFIX         ${CMAKE_SOURCE_DIR}/stm32-tools/flatbuffers
)

FetchContent_Populate(Flatbuffers)

FetchContent_GetProperties(Flatbuffers
    POPULATED Flatbuffers_POPULATED
    SOURCE_DIR FlatBuffers_SOURCE_DIR
    BINARY_DIR FlatBuffers_BINARY_DIR
)

set(FlatBuffers_FOUND TRUE)

