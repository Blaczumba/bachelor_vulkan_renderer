add_subdirectory(shader)

add_library(Pipeline pipeline.cpp graphics_pipeline.cpp compute_pipeline.cpp)

target_link_libraries(Pipeline PUBLIC Vulkan::Vulkan)
target_link_libraries(Pipeline PUBLIC LogicalDevice Renderpass Shader)

target_include_directories(Pipeline PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(Pipeline PUBLIC SHADERS_PATH="${CMAKE_SOURCE_DIR}/shaders/")