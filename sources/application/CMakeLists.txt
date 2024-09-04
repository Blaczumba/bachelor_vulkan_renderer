add_library(Application application_base.cpp offscreen_rendering_application.cpp)
# add_library(Application application_base.cpp double_screenshot_application.cpp)

target_link_libraries(Application PRIVATE Vulkan::Vulkan)
target_link_libraries(Application PUBLIC Window Instance DebugMessenger Surface PhysicalDevice LogicalDevice Renderpass Swapchain CommandBuffer Framebuffer VertexBuffer IndexBuffer UniformBuffer Pipeline Texture TinyGLTFLoader OBJLoader DescriptorSet Camera CallbackManager Screenshot Utils)
target_link_libraries(Application PRIVATE OBJLoader)

target_include_directories(Application PUBLIC ${CMAKE_SOURCE_DIR}/sources)
target_include_directories(Application PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})