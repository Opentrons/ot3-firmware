set(TARGET_MODULE_NAME "rear-startup")
if (${CMAKE_CROSSCOMPILING})
  add_subdirectory(application-startup)    
  add_subdirectory(STM32G491)
else()
  #add_subdirectory(tests)
endif()

