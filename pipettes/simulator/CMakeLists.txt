add_executable(
        pipettes-simulator
        main.cpp
)

# Set up the include directories
target_include_directories(
        pipettes-simulator
        PUBLIC ${CMAKE_SOURCE_DIR}/include)


set_target_properties(pipettes-simulator
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE)
