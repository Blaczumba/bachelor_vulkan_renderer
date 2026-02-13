add_library(OpenxrPresentation presentation.h presentation.cpp)

target_link_libraries(OpenxrPresentation PRIVATE OpenXR::headers)
target_link_libraries(OpenxrPresentation PRIVATE OpenxrSystem OpenxrSession)
target_link_libraries(OpenxrPresentation PUBLIC OpenxrGraphicsPlugin)

target_include_directories(OpenxrPresentation PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(OpenxrPresentation PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
