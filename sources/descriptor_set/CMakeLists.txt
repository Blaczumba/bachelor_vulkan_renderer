add_library(DescriptorSet descriptor_set.cpp)

target_link_libraries(DescriptorSet PRIVATE Vulkan::Vulkan)
target_link_libraries(DescriptorSet PUBLIC LogicalDevice UniformBuffer)

target_include_directories(DescriptorSet PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(DescriptorSet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})