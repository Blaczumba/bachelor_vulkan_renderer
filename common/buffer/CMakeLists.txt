add_library(CommonBuffer buffer.h buffer.cpp)

target_include_directories(CommonBuffer PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(CommonBuffer PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})