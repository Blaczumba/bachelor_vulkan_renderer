add_library(PhysicalDeviceFeatures features.cpp)

target_link_libraries(PhysicalDeviceFeatures PRIVATE Vulkan::Vulkan)

target_include_directories(PhysicalDeviceFeatures PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(PhysicalDeviceFeatures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})