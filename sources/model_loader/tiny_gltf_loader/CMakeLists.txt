add_library(TinyGLTFLoader INTERFACE tiny_gltf_loader.h)

target_link_libraries(TinyGLTFLoader INTERFACE Vulkan::Vulkan)

target_include_directories(TinyGLTFLoader INTERFACE ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(TinyGLTFLoader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(TinyGLTFLoader INTERFACE ${CMAKE_SOURCE_DIR}/external)