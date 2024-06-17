add_subdirectory(framebuffer)

add_library(Renderpass render_pass.cpp)

target_link_libraries(Renderpass PUBLIC LogicalDevice)
target_link_libraries(Renderpass PUBLIC Vulkan::Vulkan)

target_include_directories(Renderpass PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Renderpass PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
