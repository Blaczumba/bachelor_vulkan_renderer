add_library(OpenxrPlatform platform.h android_platform.h android_platform.cpp)

target_compile_definitions(OpenxrPlatform PUBLIC XR_USE_PLATFORM_ANDROID)
target_link_libraries(OpenxrPlatform PUBLIC OpenXR::headers NativeAppGlue android spdlog)
target_link_libraries(OpenxrPlatform PRIVATE OpenxrGraphicsPlugin) # TODO: this theoretically is not needed but it looks like it demads it when linking.

target_include_directories(OpenxrPlatform PRIVATE ${PROJECT_SOURCE_DIR}/openxr ${PROJECT_SOURCE_DIR}/sources)
target_include_directories(OpenxrPlatform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
