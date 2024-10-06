add_library(ThreadPool thread_pool.cpp)

target_include_directories(ThreadPool PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(ThreadPool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})