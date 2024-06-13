add_library(LogicalDevice logical_device.cpp)

target_link_libraries(LogicalDevice PUBLIC PhysicalDevice)

target_include_directories(LogicalDevice PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(LogicalDevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
