add_library(UniformBuffer uniform_buffer.cpp push_constants.cpp)

target_link_libraries(UniformBuffer PUBLIC Vulkan::Vulkan)
target_link_libraries(UniformBuffer PUBLIC LogicalDevice Texture)

target_include_directories(UniformBuffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(UniformBuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})