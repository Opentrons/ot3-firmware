add_library(common-simulation STATIC
        spi.cpp
        i2c_sim.cpp
        app_update.cpp)

target_link_libraries(common-simulation PUBLIC can-core)

if(NOT DEFINED ENV{DISABLE_LOGGING})
    target_compile_definitions(common-simulation PUBLIC ENABLE_LOGGING)
endif()

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


# Logging module requires FreeRTOS, which means it cannot be compiled into "common-simulation" shared lib.
function(target_enable_logging TARGET)
    if(NOT DEFINED ENV{DISABLE_LOGGING})
        target_sources(${TARGET} PUBLIC
                ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/logging.cpp)
        target_compile_definitions(${TARGET} PUBLIC ENABLE_LOGGING)
    endif()
endfunction()
