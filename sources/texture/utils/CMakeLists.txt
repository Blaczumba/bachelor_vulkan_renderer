add_library(TextureUtils texture_utils.cpp)

target_link_libraries(TextureUtils PUBLIC Vulkan::Vulkan)

target_include_directories(TextureUtils PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(TextureUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
