set(TEST_NAME VulkanTests)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(${TEST_NAME} test_vulkan.cpp)
target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest GTest::gtest_main)
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/glm)

target_include_directories(${TEST_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(${TEST_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})