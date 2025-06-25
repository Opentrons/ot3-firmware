#[=======================================================================[.rst:
ClangTidy
---------

This is a CMake module providing macros and functions to handle calling
ClangTidy on our cross-compiled sources, which is a little involved.

Usage
+++++

To use this module, make sure you're setting the cmake module path to this
directory and call
```
include(ClangTidy)
```

Then, you can use the macros defined in here.


Provided Macros
++++++++++++++++

add_clang_tidy_target(
  TARGET targetname
  LINT_SOURCES sources...
  [NO_ADD_TO_LINT_TARGET]
  )

This macro creates a custom target that calls clang-tidy. LINT_SOURCES should be a list of source
files that should be linted; it should not include files that shouldn't be checked, which is
generally C files and files that are entirely HAL calls that clang-tidy hates.

The macro will also add the target to LINT_TARGETS which is assumed to exist in parent scope, unless
NO_ADD_TO_LINT_TARGETS is set.

#]=======================================================================]

macro(add_clang_tidy_target)
  set(_actt_onevalue TARGET_NAME)
  set(_actt_multivalue LINT_SOURCES)
  set(_actt_options NO_ADD_TO_LINT_TARGET)
  cmake_parse_arguments(_actt "${_actt_options}" "${_actt_onevalue}" "${_actt_multivalue}" ${ARGN})
  if(NOT _actt_TARGET_NAME)
    message(FATAL_ERROR "No TARGET_NAME passed to add_clang_tidy_target")
  endif()
  if(NOT _actt_LINT_SOURCES)
    message(FATAL_ERROR "No LINT_SOURCES passed to add_clang_tidy_target")
  endif()
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
  set(_actt_clang_extra_args_${_actt_TARGET_NAME} ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  list(TRANSFORM _actt_clang_extra_args_${_actt_TARGET_NAME} PREPEND --extra-arg=-I)
  # This helps with clang accepting what GCC accepts around the implementations of the message queue
  list(APPEND _actt_clang_extra_args_${_actt_TARGET_NAME} "--extra-arg=-frelaxed-template-template-args")
  # This helps with modern clang-tidy not liking the gcc specs setting for cross builds
  list(APPEND _actt_clang_extra_args_${_actt_TARGET_NAME}  "-Wno-unused-command-line-argument")
  list(APPEND _actt_clang_extra_args_${_actt_TARGET_NAME}  "-Wno-error=unused-command-line-argument")
  add_custom_target(${_actt_TARGET_NAME}
    ALL
    COMMAND ${Clang_CLANGTIDY_EXECUTABLE} "--extra-arg=-DOPENTRONS_CLANG_TIDY_WORKAROUND_44178" ${_actt_clang_extra_args_${_actt_TARGET_NAME}} -p ${CMAKE_BINARY_DIR} ${_actt_LINT_SOURCES})
  if (NOT ${_actt_NO_ADD_TO_LINT_TARGET})
    list(APPEND LINT_TARGETS ${_actt_TARGET_NAME})
    set(LINT_TARGETS ${LINT_TARGETS} PARENT_SCOPE)
    message(STATUS "${_actt_TARGET_NAME} created (part of the lint target)")
  elseif()
    message(STATUS "${_actt_TARGET_NAME} created (not part of the lint target)")
  endif()
endmacro()
