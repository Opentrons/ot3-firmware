add_library(common-simulation STATIC
            app_update.cpp
            logging.cpp
            state_manager.cpp)

target_link_libraries(common-simulation PUBLIC can-core Boost::boost Boost::date_time pthread)

add_dependencies(common-simulation state_manager)

target_compile_definitions(common-simulation PUBLIC ENABLE_LOGGING)

add_dependencies(common-simulation state-manager-headers)

set_target_properties(common-simulation
        PROPERTIES CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_include_directories(common-simulation PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/cpp-utils/include
        ${CMAKE_BINARY_DIR} # To include state manager headers
        )

target_compile_options(common-simulation
        PRIVATE
        -Wall
        -Werror
        -Wextra
        -Wno-missing-field-initializers
        $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
        $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
        $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
        $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
)
