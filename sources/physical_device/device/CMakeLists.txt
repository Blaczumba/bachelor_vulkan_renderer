add_library(PhysicalDevice physical_device.cpp)

target_link_libraries(PhysicalDevice PUBLIC Surface PhysicalDevicePropertyManager)

target_include_directories(PhysicalDevice PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(PhysicalDevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})