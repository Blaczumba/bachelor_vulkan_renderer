add_library(SwapchainUtils swapchain_utils.cpp)

target_link_libraries(SwapchainUtils PUBLIC Vulkan::Vulkan glfw)

target_include_directories(SwapchainUtils PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(SwapchainUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
