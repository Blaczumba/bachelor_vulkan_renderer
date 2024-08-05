add_library(PhysicalDevice physical_device.cpp property_manager.cpp)

target_link_libraries(PhysicalDevice PUBLIC Surface LogicalDevice)

target_include_directories(PhysicalDevice PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(PhysicalDevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})