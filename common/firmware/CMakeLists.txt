# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

# Add source files that should be checked by clang-tidy here
set(COMMON_FW_LINTABLE_SRCS
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/freertos_comms_task.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/freertos_task.cpp)

add_executable(common
  ${COMMON_FW_LINTABLE_SRCS})

target_link_options(common
  PRIVATE
  "LINKER:--print-memory-usage"
  "LINKER:--error-unresolved-symbols")

set_target_properties(common
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED TRUE
             C_STANDARD 11
             C_STANDARD_REQUIRED TRUE)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

target_compile_options(common
  PUBLIC
  -Wall
  -Werror
  $<$<COMPILE_LANGUAGE:CXX>:-Weffc++>
  $<$<COMPILE_LANGUAGE:CXX>:-Wreorder>
  $<$<COMPILE_LANGUAGE:CXX>:-Wsign-promo>
  $<$<COMPILE_LANGUAGE:CXX>:-Wextra-semi>
  $<$<COMPILE_LANGUAGE:CXX>:-Wctor-dtor-privacy>
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

target_link_libraries(
  common
  LINK_PUBLIC
  system)


find_package(Clang)

# # runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# # which is a catch-all static analyzer/linter
# # the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# # An odd thing about this target is that it requires the existance of a compiledb, which
# # is produced when you build, and may change if you change compilation options, so in a way
# # it depends on a build. But we also want to be able to run this when there wasn't a successful
# # build, so there's no explicit dependency set.
# # This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# # own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# # include directories. So we can use the actually fairly cool transform command to turn them all into
# # extra-arg invocations and it'll figure it out.
# set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
# list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# # This helps with clang accepting what GCC accepts around the implementations of the message queue
# list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(common-lint
ALL
COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${PIPETTE_FW_LINTABLE_SRCS} ${core_sources} --config=)

# Runs cross-gdb (since CMAKE_CROSSCOMPILING_EMULATOR is set in an
# arguable misuse of the concept) to the appropriate cross-gdb with
# remote-target. You should make sure st-util is running; that's not
# done here because it won't be multi-os compatible, and also it
# should be running the entire time and that's tough to accomplish
# in a custom command
add_custom_target(common-debug
COMMAND common
USES_TERMINAL
)
