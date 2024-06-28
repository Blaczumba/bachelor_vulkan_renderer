add_library(TinyGLTFLoader tiny_gltf_loader.cpp)

target_link_libraries(TinyGLTFLoader PUBLIC Vulkan::Vulkan)

target_include_directories(TinyGLTFLoader PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(TinyGLTFLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(TinyGLTFLoader PRIVATE "${CMAKE_SOURCE_DIR}/external")