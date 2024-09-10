add_library(ECSRegistry registry.cpp)

# target_link_libraries(ECSRegistry PUBLIC ${Boost_LIBRARIES})

target_include_directories(ECSRegistry PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(ECSRegistry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})