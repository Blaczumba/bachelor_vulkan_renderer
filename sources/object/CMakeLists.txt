add_library(Object object.cpp volumes.cpp)

target_link_libraries(Object VertexBuffer IndexBuffer DescriptorSet Scene)

target_include_directories(Object PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Object PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
