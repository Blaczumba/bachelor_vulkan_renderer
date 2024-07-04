add_library(Image image.cpp)

target_link_libraries(Image PUBLIC Vulkan::Vulkan glfw)

target_include_directories(Image PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(texture)

add_library(VertexBuffer "vertex_buffer.cpp")

target_link_libraries(VertexBuffer PUBLIC Vulkan::Vulkan)
target_link_libraries(VertexBuffer PUBLIC LogicalDevice CommandBuffer)

target_include_directories(VertexBuffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(VertexBuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(IndexBuffer "index_buffer.cpp")

target_link_libraries(IndexBuffer PUBLIC Vulkan::Vulkan)
target_link_libraries(IndexBuffer PUBLIC LogicalDevice CommandBuffer)

target_include_directories(IndexBuffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(IndexBuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(UniformBuffer "uniform_buffer.cpp")

target_link_libraries(UniformBuffer PUBLIC Vulkan::Vulkan)
target_link_libraries(UniformBuffer PUBLIC LogicalDevice Texture)

target_include_directories(UniformBuffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(UniformBuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})