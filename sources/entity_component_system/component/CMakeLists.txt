add_library(Components component.cpp "transform.h" "component_pool.h")

target_link_libraries(Components PUBLIC VertexBuffer IndexBuffer Texture)

target_include_directories(Components PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})