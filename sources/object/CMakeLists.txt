add_library(Object object.cpp)

target_link_libraries(Object VertexBuffer IndexBuffer DescriptorSet Scene Entity)

target_include_directories(Object PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Object PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
