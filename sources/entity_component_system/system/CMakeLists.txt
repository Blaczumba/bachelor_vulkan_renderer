add_library(ComponentSystem movement_system.cpp)

target_include_directories(ComponentSystem PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(ComponentSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})