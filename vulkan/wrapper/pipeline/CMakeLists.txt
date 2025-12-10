add_library(Pipeline pipeline.cpp shader.cpp graphics_pipeline_builder.h graphics_pipeline_builder.cpp pipeline_layout.h pipeline_layout.cpp)

target_link_libraries(Pipeline PUBLIC Vulkan::Vulkan)
target_link_libraries(Pipeline PUBLIC LogicalDevice Renderpass)

target_include_directories(Pipeline PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(Pipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: refactor
if (ANDROID)
    target_compile_definitions(Pipeline PUBLIC SHADERS_PATH="shaders/")
else ()
    target_compile_definitions(Pipeline PUBLIC SHADERS_PATH="${PROJECT_SOURCE_DIR}/shaders/")
endif ()