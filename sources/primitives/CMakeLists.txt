add_library(Primitives geometry.cpp)

# target_link_libraries(Primitives)

target_include_directories(Primitives PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Primitives PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})