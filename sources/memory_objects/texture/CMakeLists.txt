set(KTX_DIR ${CMAKE_SOURCE_DIR}/external/ktx)
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)


add_library(Texture texture.cpp texture_2D.cpp texture_2D_depth.cpp texture_2D_color.cpp  texture_2D_image.cpp texture_cubemap.cpp texture_2D_shadow.cpp texture_sampler.cpp ${KTX_SOURCES})

target_link_libraries(Texture PUBLIC Vulkan::Vulkan)
target_link_libraries(Texture PUBLIC LogicalDevice CommandBuffer Image)

target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/external/ktx/include)
target_include_directories(Texture PUBLIC ${CMAKE_SOURCE_DIR}/external/ktx/other_include)
target_include_directories(Texture PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})