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

find_package(Boost REQUIRED COMPONENTS ${BOOST_COMPONENTS})

if(NOT Boost_FOUND)
    message(WARNING "Boost not found!")
else()
    add_definitions(-DBOOST_ENABLED)

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME OFF)

    set(BOOST_COMPONENTS filesystem system)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building in Release mode.")
else()
    message(STATUS "Building in Debug mode.")
    add_definitions(-DVALIDATION_LAYERS_ENABLED)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/external/glfw)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/tinyobjloader)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest)

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
elseif(UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -pedantic -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
endif()

add_subdirectory(sources)
