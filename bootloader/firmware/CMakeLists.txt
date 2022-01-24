# This CMakeLists.txt handles everything that is compiled only when
# cross-compiling, like the board support packages and special options.

set(COMMON_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/common/firmware")
set(CAN_EXECUTABLE_DIR "${CMAKE_SOURCE_DIR}/can/firmware")

# Add source files that should be checked by clang-tidy here
set(BOOTLOADER_FW_LINTABLE_SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/main.c
        )

set(BOOTLOADER_FW_NON_LINTABLE_SRCS
        ${COMMON_EXECUTABLE_DIR}/errors/errors.c
        ${CAN_EXECUTABLE_DIR}/utils.c
        )

if (${ARM_ARCH_TYPE} STREQUAL "cortex-m4")
    add_subdirectory(stm32G4)
elseif (${ARM_ARCH_TYPE} STREQUAL "cortex-m33")
    add_subdirectory(stm32L5)
endif()

find_package(Clang)

# runs clang-tidy https://releases.llvm.org/11.0.1/tools/clang/tools/extra/docs/clang-tidy/index.html
# which is a catch-all static analyzer/linter
# the empty --config= tells clang-tidy to use the .clang-tidy file in the top level
# An odd thing about this target is that it requires the existance of a compiledb, which
# is produced when you build, and may change if you change compilation options, so in a way
# it depends on a build. But we also want to be able to run this when there wasn't a successful
# build, so there's no explicit dependency set.
# This awful transform is required because the implicit includes that gcc knows how to find (e.g. its
# own implementation of the STL) don't get added to the compile db that clang-tidy uses to figure out
# include directories. So we can use the actually fairly cool transform command to turn them all into
# extra-arg invocations and it'll figure it out.
set(CLANG_EXTRA_ARGS ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
list(TRANSFORM CLANG_EXTRA_ARGS PREPEND --extra-arg=-I)
# This helps with clang accepting what GCC accepts around the implementations of the message queue
list(APPEND CLANG_EXTRA_ARGS "--extra-arg=-frelaxed-template-template-args")
add_custom_target(bootloader-lint
        ALL
        COMMAND ${Clang_CLANGTIDY_EXECUTABLE} ${CLANG_EXTRA_ARGS} -p ${CMAKE_BINARY_DIR} ${BOOTLOADER_FW_LINTABLE_SRCS} ${CORE_LINTABLE_SOURCES})
list(APPEND LINT_TARGETS bootloader-lint)
set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
