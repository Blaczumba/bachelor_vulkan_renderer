add_subdirectory(framebuffer)
add_subdirectory(attachment)

add_library(Renderpass render_pass.cpp subpass/subpass.h subpass/subpass.cpp "attachment/attachment_factory.cpp" "attachment/attachment_factory.h")

target_link_libraries(Renderpass PUBLIC LogicalDevice)
target_link_libraries(Renderpass PUBLIC Vulkan::Vulkan)

target_include_directories(Renderpass PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Renderpass PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
