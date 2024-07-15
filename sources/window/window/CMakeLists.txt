add_library(Window window.cpp)
add_library(WindowGLFW window_glfw.cpp)

target_link_libraries(Window PUBLIC Vulkan::Vulkan)
target_link_libraries(WindowGLFW PUBLIC Window glfw)

target_include_directories(Window PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Window PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(WindowGLFW PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(WindowGLFW PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
