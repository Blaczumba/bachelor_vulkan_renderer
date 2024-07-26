add_library(DescriptorSet descriptor_set.cpp descriptor_pool.cpp descriptor_set_layout.cpp "descriptor_sets_aggregator.cpp" "descriptor_sets_aggregator.h")

target_link_libraries(DescriptorSet PRIVATE Vulkan::Vulkan)
target_link_libraries(DescriptorSet PUBLIC LogicalDevice UniformBuffer Pipeline)

target_include_directories(DescriptorSet PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(DescriptorSet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})