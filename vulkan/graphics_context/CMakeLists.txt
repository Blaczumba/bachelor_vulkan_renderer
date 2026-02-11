add_library(VulkanLayer graphics_context.h presentation.h presentation.cpp graphics_context.cpp)

target_link_libraries(VulkanLayer PUBLIC Vulkan::Vulkan)
target_link_libraries(VulkanLayer PUBLIC CommonModelLoader Instance PhysicalDevice LogicalDevice Surface Swapchain)

target_include_directories(VulkanLayer PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(VulkanLayer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
