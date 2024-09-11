add_library(ECSRegistry registry.cpp)

if(Boost_FOUND)
	target_link_libraries(ECSRegistry PUBLIC ${Boost_LIBRARIES})
endif()

target_include_directories(ECSRegistry PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(ECSRegistry PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})