add_library(Image image.cpp)

target_link_libraries(Image PUBLIC Vulkan::Vulkan)
target_link_libraries(Image PUBLIC CommandBuffer)

target_include_directories(Image PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Image PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(Buffer buffer.cpp)

target_link_libraries(Buffer PUBLIC Vulkan::Vulkan)

target_include_directories(Buffer PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Buffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

