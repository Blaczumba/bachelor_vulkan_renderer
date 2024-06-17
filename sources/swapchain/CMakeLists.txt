add_library(SwapChainUtils swapchain_utils.cpp)

target_link_libraries(SwapChainUtils PUBLIC Vulkan::Vulkan glfw)

target_include_directories(SwapChainUtils PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(SwapChainUtils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Swapchain swapchain.cpp)

target_link_libraries(Swapchain PUBLIC LogicalDevice SwapChainUtils)

target_include_directories(Swapchain PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Swapchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
