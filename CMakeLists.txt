cmake_minimum_required(VERSION 3.19)

set(CMAKE_PROJECT_NAME VulkanProject)

project(${CMAKE_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode.")
else()
    message(STATUS "Building in Debug mode.")
endif()

add_subdirectory(external/glfw)
add_subdirectory(external/tinyobjloader)
add_subdirectory(external/googletest)

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -pedantic -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

add_executable(${CMAKE_PROJECT_NAME} main.cpp)

if(WIN32)
    file(CHMOD "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile_shaders.bat" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

    add_custom_target(
    CompileShaders
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile_shaders.bat"
    COMMENT "Compiling shaders..."
    )
elseif(UNIX)
    file(CHMOD "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile_shaders.sh" PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ)

    add_custom_target(
    CompileShaders
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/shaders/compile_shaders.sh"
    COMMENT "Compiling shaders..."
    )
endif()

add_dependencies(${CMAKE_PROJECT_NAME} CompileShaders)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/")
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC MODELS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/models/")

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw tinyobjloader)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/glm")
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/external/tinygltf")

add_subdirectory(tests)

