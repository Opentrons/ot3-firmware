add_library(motor-utils STATIC
    motor_hardware_interface.cpp
    stall_check.cpp
    types.cpp
    utils.cpp)

set_target_properties(motor-utils
    PROPERTIES CXX_STANDARD 20
    CXX_STANDARD_REQUIRED TRUE)

target_compile_options(motor-utils
    PUBLIC
    -Wall
    -Werror
    -Wno-missing-field-initializers
    -Wextra
    $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
    $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
    $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
    $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    )

target_include_directories(motor-utils PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../../include)

add_coverage(motor-utils)
