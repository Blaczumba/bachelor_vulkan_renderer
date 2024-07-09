add_library(Camera fps_camera.cpp)

target_link_libraries(Camera PUBLIC glfw)

target_include_directories(Camera PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Camera PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})