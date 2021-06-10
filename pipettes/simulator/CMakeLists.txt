# This CMakeLists.txt handles compiling all the parts of the pipettes
# module that are portable between host and cross compilation as a static
# library. It is included in both host and cross configurations.

add_executable(pipettes-simulator
  main.cpp
)

set_target_properties(pipettes-simulator
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED TRUE)

target_include_directories(pipettes-simulator
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_include_directories(pipettes-simulator
  PUBLIC ${FlatBuffers_SOURCE_DIR}/include)

# Now simply link against flatbuffers as needed to your already declared target.
# The flatbuffers target carry header search path automatically if CMake > 2.8.11.
target_link_libraries(pipettes-simulator PRIVATE flatbuffers)

target_compile_options(pipettes-simulator
  PRIVATE
  -Wall
  # -Werror
  -Weffc++
  -Wreorder
  -Wsign-promo
  -Wextra-semi
  -Wctor-dtor-privacy
  -fno-rtti
  )