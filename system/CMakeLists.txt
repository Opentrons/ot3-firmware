# add_subdirectory(src)
add_subdirectory(firmware)

if (NOT ${CMAKE_CROSSCOMPILING})
  # add_subdirectory(tests)
endif()
