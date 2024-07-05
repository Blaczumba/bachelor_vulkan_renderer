add_library(Image ../image.cpp image_color.cpp image_depth.cpp)

target_link_libraries(Image PUBLIC Vulkan::Vulkan)
target_link_libraries(Image PUBLIC LogicalDevice)

target_include_directories(Image PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})