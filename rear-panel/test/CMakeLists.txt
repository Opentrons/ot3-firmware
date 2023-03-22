# this CMakeLists.txt file is only used when host-compiling to build tests
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
include(AddBuildAndTestTarget)

add_executable(
        rear-panel
        test_main.cpp
        test_messages.cpp
        test_light_animation.cpp
)

# TODO(FS, 2023/03/22): We can't actually add in the entire rear-panel/core 
# function since it uses FreeRTOS-specific headers... so instead I guess we 
# can manually add source files? Fix this in the future.
target_sources(rear-panel PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/lights/animation_math.cpp
)

target_include_directories(rear-panel PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
set_target_properties(rear-panel
        PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE)

target_compile_options(rear-panel
        PUBLIC
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

add_revision(TARGET rear-panel REVISION b1)

target_link_libraries(rear-panel PUBLIC Catch2::Catch2)

catch_discover_tests(rear-panel)
add_build_and_test_target(rear-panel)

add_coverage(rear-panel)
