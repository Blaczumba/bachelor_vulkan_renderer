add_subdirectory(features)

add_library(Swapchain swapchain.cpp)

target_link_libraries(Swapchain PUBLIC LogicalDevice Window SwapChainUtils Buffers)

target_include_directories(Swapchain PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Swapchain PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
