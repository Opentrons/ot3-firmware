add_library(common-core STATIC tool_detection.cpp)
target_include_directories(common-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(common-core PUBLIC can-core)

set_target_properties(common-core
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(common-core
        PRIVATE
        -Wall
        -Werror
        -Weffc++
        -Wreorder
        -Wsign-promo
        -Wextra-semi
        -Wctor-dtor-privacy
        -fno-rtti
        )
