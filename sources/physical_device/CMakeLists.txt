add_library(PhysicalDevice physical_device.cpp "features.h" "features.cpp")

target_link_libraries(PhysicalDevice PUBLIC Instance Surface)

target_include_directories(PhysicalDevice PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(PhysicalDevice PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})