add_library(motor-core STATIC utils.cpp)
target_include_directories(motor-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
