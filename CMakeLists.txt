cmake_minimum_required(VERSION 3.14)

set(CMAKE_PROJECT_NAME VulkanProject)

project(${CMAKE_PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found!")
endif()

add_subdirectory(external/glfw)
add_subdirectory(external/googletest)
add_subdirectory(external/glm)

add_executable(${CMAKE_PROJECT_NAME} main.cpp)
target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Vulkan::Vulkan glfw glm)

add_subdirectory(tests)

