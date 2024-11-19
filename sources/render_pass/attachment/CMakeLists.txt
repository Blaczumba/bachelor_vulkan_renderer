add_library(AttachmentFactory attachment_factory.cpp)

target_link_libraries(AttachmentFactory PUBLIC Vulkan::Vulkan)

target_include_directories(AttachmentFactory PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(AttachmentFactory PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})