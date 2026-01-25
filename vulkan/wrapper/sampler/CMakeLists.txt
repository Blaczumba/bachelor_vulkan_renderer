add_library(Sampler sampler.h sampler.cpp)

target_link_libraries(Sampler PUBLIC Vulkan::Vulkan)
target_link_libraries(Sampler PUBLIC LogicalDevice)

target_include_directories(Sampler PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(Sampler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
