add_library(Utils utils.cpp)

target_link_libraries(Utils PUBLIC LogicalDevice Texture)

target_include_directories(Utils PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
