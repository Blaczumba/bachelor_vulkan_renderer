add_library(Components component_pool.cpp "velocity.h" "position.h")

target_link_libraries(Components PUBLIC VertexBuffer IndexBuffer Texture)

target_include_directories(Components PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})