add_library(Subpass subpass.cpp)

target_link_libraries(Subpass PUBLIC Attachment)
target_link_libraries(Subpass PUBLIC Vulkan::Vulkan)

target_include_directories(Subpass PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Subpass PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
