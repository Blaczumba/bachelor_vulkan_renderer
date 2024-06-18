add_library(SwapChainUtils swapchain_utils.cpp)

target_link_libraries(SwapChainUtils PUBLIC Vulkan::Vulkan glfw)

target_include_directories(SwapChainUtils PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(SwapChainUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})