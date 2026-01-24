add_library(AssetManager asset_manager.h asset_manager.cpp pipeline_manager.h pipeline_manager.cpp hasher.h gpu_buffer_manager.h gpu_buffer_manager.cpp util.h)

target_link_libraries(AssetManager PUBLIC Vulkan::Vulkan)
target_link_libraries(AssetManager PUBLIC CommonModelLoader LogicalDevice Buffer CommonUtil VulkanWrapperUtil)

target_include_directories(AssetManager PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(AssetManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: refactor
if (ANDROID)
    target_compile_definitions(AssetManager PUBLIC SHADERS_PATH="shaders/")
else ()
    target_compile_definitions(AssetManager PUBLIC SHADERS_PATH="${PROJECT_SOURCE_DIR}/shaders/")
endif ()