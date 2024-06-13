add_library(Surface surface.cpp)

target_link_libraries(Surface PRIVATE Instance Window)

target_include_directories(Surface PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Surface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
