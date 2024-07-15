add_library(CallbackManager callback_manager.cpp fps_callback_manager.cpp)

target_link_libraries(CallbackManager PUBLIC glfw)
target_link_libraries(CallbackManager PUBLIC WindowGLFW CallbackObserver)

target_include_directories(CallbackManager PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(CallbackManager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
