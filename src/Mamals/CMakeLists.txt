add_library(Mamals INTERFACE)
target_include_directories(Mamals INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(Mamals INTERFACE project_properties)

add_subdirectory(Canidae)
add_subdirectory(Felinae)
