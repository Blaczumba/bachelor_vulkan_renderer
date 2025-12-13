add_library(LibThread INTERFACE worker.h)

target_include_directories(LibThread INTERFACE ${PROJECT_SOURCE_DIR})
target_include_directories(LibThread INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})