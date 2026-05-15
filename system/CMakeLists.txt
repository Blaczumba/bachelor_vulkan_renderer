add_library(System system.h system.cpp)

target_link_libraries(System PRIVATE CommonAbstractions)

target_include_directories(System PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(System PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})