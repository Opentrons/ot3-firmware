add_library(motor-utils STATIC utils.cpp)
target_include_directories(motor-utils PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../include)