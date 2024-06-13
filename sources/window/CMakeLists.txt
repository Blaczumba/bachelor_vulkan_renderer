add_library(Window window.cpp)

target_link_libraries(Window PUBLIC Vulkan::Vulkan glfw)

target_include_directories(Window PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
