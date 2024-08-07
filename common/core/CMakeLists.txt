add_custom_target(generate_version
        ${CMAKE_COMMAND} -DGENERATE_VERSION_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/version.c)

add_library(version-lib STATIC ${CMAKE_CURRENT_BINARY_DIR}/version.c)

target_compile_options(version-lib
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
target_include_directories(version-lib PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_dependencies(version-lib generate_version)

add_library(common-core STATIC ${CMAKE_CURRENT_BINARY_DIR}/version.c)
target_include_directories(common-core PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(common-core PUBLIC can-core version-lib)


set_target_properties(common-core
        PROPERTIES CXX_STANDARD 20
                   CXX_STANDARD_REQUIRED TRUE)

target_compile_options(common-core
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

add_coverage(common-core)

function(add_revision)
  set(_ar_options)
  set(_ar_onevalue TARGET REVISION)
  set(_ar_multivalue)
  cmake_parse_arguments(_ar "${_ar_options}" "${_ar_onevalue}" "${_ar_multivalue}" ${ARGN})
  message(STATUS "add_revision has target ${_ar_TARGET} and revision ${_ar_REVISION}")
  string(SUBSTRING ${_ar_REVISION} 0 1 PRIMARY_REVISION)
  string(SUBSTRING ${_ar_REVISION} 1 1 SECONDARY_REVISION)
  set(TERTIARY_FLAG "0x00")


  configure_file(${CMAKE_SOURCE_DIR}/common/core/revision.c.in ${CMAKE_CURRENT_BINARY_DIR}/${_ar_TARGET}_revision.c)

  target_sources(${_ar_TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/${_ar_TARGET}_revision.c)

  target_compile_definitions(${_ar_TARGET} PUBLIC
      PCBA_REVISION="${_ar_REVISION}"
      PCBA_PRIMARY_REVISION='${PRIMARY_REVISION}'
      PCBA_SECONDARY_REVISION='${SECONDARY_REVISION}')
  set(_add_revision_PRIMARY_REVISION ${PRIMARY_REVISION} PARENT_SCOPE)
  set(_add_revision_SECONDARY_REVISION ${SECONDARY_REVISION} PARENT_SCOPE)
endfunction()
