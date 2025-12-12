add_library(LibThread worker.h worker.cpp)

target_include_directories(LibThread PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(LibThread PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})