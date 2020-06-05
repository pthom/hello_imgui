add_library(Felinae cat.cpp)
target_include_directories(Felinae
    PUBLIC 
        $<INSTALL_INTERFACE:include>    
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
target_link_libraries(Felinae PUBLIC project_properties project_warnings)
target_link_libraries(Felinae PUBLIC Mamals)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
  add_executable(cat_test cat.test.cpp)
  target_link_libraries(cat_test PRIVATE main_test Felinae)
  add_test(NAME cat_test COMMAND cat_test)
endif()
