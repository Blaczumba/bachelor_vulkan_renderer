add_library(Framebuffer framebuffer.cpp "framebuffer_from_swapchain.h" "framebuffer_from_swapchain.cpp" "framebuffer_from_textures.cpp" "framebuffer_from_textures.h")

target_link_libraries(Framebuffer PUBLIC LogicalDevice Swapchain Image)
target_link_libraries(Framebuffer PUBLIC Vulkan::Vulkan)

target_include_directories(Framebuffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Framebuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
