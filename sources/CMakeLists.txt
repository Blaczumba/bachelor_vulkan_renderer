add_subdirectory(window)
add_subdirectory(instance)
add_subdirectory(debug_messenger)
add_subdirectory(surface)
add_subdirectory(physical_device)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

if(WIN32)
    set(SHADER_COMPILE_SCRIPT "${CMAKE_SOURCE_DIR}/shaders/compile_shaders.bat")
elseif(UNIX)
    set(SHADER_COMPILE_SCRIPT "${CMAKE_SOURCE_DIR}/shaders/compile_shaders.sh")
endif()

file(CHMOD ${SHADER_COMPILE_SCRIPT} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

    add_custom_target(
    CompileShaders
    COMMAND ${SHADER_COMPILE_SCRIPT}
    COMMENT "Compiling shaders..."
    )

add_dependencies(${CMAKE_PROJECT_NAME} CompileShaders)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SHADERS_PATH="${CMAKE_SOURCE_DIR}/shaders/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC TEXTURES_PATH="${CMAKE_SOURCE_DIR}/assets/textures/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC MODELS_PATH="${CMAKE_SOURCE_DIR}/assets/models/")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw tinyobjloader Window Instance DebugMessenger Surface PhysicalDevice)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/tinygltf")

add_subdirectory(tests)