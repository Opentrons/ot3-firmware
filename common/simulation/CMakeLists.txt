add_library(common-simulation STATIC spi.cpp eeprom.cpp)

target_link_libraries(common-simulation PUBLIC can-core)

target_compile_definitions(common-simulation PUBLIC ENABLE_LOGGING)

set_target_properties(common-simulation
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(common-simulation
        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_compile_options(common-simulation
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
