add_library(Texture "texture_2D.cpp")

target_link_libraries(Texture PUBLIC Vulkan::Vulkan)
target_link_libraries(Texture PUBLIC LogicalDevice CommandBuffer Image)

target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Texture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Texture PRIVATE "${CMAKE_SOURCE_DIR}/external/stb_image")