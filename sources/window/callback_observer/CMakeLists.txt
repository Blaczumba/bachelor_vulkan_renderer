add_library(CallbackObserver callback_observer.cpp)

# target_link_libraries(CallbackObserver PUBLIC CallbackManager)

target_include_directories(CallbackObserver PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(CallbackObserver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})