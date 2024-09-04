add_library(ECS entity_manager.cpp entity_component_system.cpp archetype.cpp)

target_include_directories(ECS PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(ECS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
