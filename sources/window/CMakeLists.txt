add_library(Window window.cpp)

target_link_libraries(Window PRIVATE Vulkan::Vulkan glfw)

target_include_directories(Window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
