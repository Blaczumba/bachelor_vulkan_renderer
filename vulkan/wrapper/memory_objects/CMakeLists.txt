add_library(Buffer
	buffer.h buffer.cpp
	texture.h texture.cpp
	memory_objects_lib.h memory_objects_lib.cpp)
target_link_libraries(Buffer PUBLIC Vulkan::Vulkan)
target_link_libraries(Buffer PUBLIC LogicalDevice CommandBuffer)

target_include_directories(Buffer PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(Buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
