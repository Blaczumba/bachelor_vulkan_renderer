add_library(CommonAbstractions INTERFACE contexts.h graphics_context.h hardware_layer.h presentation.h)

target_link_libraries(CommonAbstractions INTERFACE glm::glm)

target_include_directories(CommonAbstractions INTERFACE ${PROJECT_SOURCE_DIR})
target_include_directories(CommonAbstractions INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})