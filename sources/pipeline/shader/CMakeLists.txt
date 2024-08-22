add_library(Shader shader.cpp shader_utils.cpp)

target_link_libraries(Shader PUBLIC Vulkan::Vulkan)
target_link_libraries(Shader PUBLIC LogicalDevice)

target_include_directories(Shader PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Shader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
