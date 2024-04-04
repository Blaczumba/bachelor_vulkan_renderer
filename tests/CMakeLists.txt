set(TEST_NAME vulkan_tests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

if(WIN32)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:msvcrtd")
endif()

add_executable(${TEST_NAME} test_vulkan.cpp)
target_link_libraries(${TEST_NAME} PRIVATE glm GTest::gtest GTest::gtest_main)