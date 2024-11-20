add_library(Primitives geometry.cpp primitives.h vk_primitives.h)

target_include_directories(Primitives PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Primitives PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})