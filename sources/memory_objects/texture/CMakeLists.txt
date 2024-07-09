add_library(Texture texture.cpp texture_2D.cpp texture_2D_depth.cpp texture_2D_color.cpp texture_2D_sampler.cpp)

target_link_libraries(Texture PUBLIC Vulkan::Vulkan)
target_link_libraries(Texture PUBLIC LogicalDevice CommandBuffer Image)

target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Texture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})