#pragma once

#include "application_base.h"

#include "camera/fps_camera.h"
#include "command_buffer/command_buffer.h"
#include "descriptor_set/descriptor_pool.h"
#include "descriptor_set/descriptor_set.h"
#include "descriptor_set/descriptor_set_layout.h"
#include "entity_component_system/system/movement_system.h"
#include "memory_objects/index_buffer.h"
#include "memory_objects/texture/texture.h"
#include "memory_objects/texture/texture_2D_color.h"
#include "memory_objects/texture/texture_2D_depth.h"
#include "memory_objects/texture/texture_2D_image.h"
#include "memory_objects/texture/texture_2D_shadow.h"
#include "memory_objects/texture/texture_cubemap.h"
#include "memory_objects/uniform_buffer/push_constants.h"
#include "memory_objects/uniform_buffer/uniform_buffer.h"
#include "memory_objects/vertex_buffer.h"
#include "model_loader/obj_loader/obj_loader.h"
#include "object/object.h"
#include "render_pass/framebuffer/framebuffer.h"
#include "render_pass/render_pass.h"
#include "scene/octree/octree.h"
#include "screenshot/screenshot.h"
#include "thread_pool/thread_pool.h"
#include "pipeline/graphics_pipeline.h"
#include "window/callback_manager/fps_callback_manager.h"

#include <unordered_map>

class SingleApp : public ApplicationBase {
    std::vector<VertexData<VertexPTNT, uint16_t>> _newVertexDataTBN;
    std::vector<std::shared_ptr<Texture2DImage>> _textures;
    std::unordered_map<std::string, std::shared_ptr<UniformBufferTexture>> _uniformMap;
    std::unordered_map<std::string, std::shared_ptr<VertexBuffer>> _vertexBufferMap;
    std::unordered_map<std::string, std::shared_ptr<IndexBuffer>> _indexBufferMap;
    std::unordered_map<Entity, uint32_t> _entityToIndex;
    std::unordered_map<Entity, std::unique_ptr<DescriptorSet>> _entitytoDescriptorSet;
    std::vector<Object> _objects;
    std::unique_ptr<Octree> _octree;
    Registry _registry;

    std::shared_ptr<Renderpass> _renderPass;
    std::vector<std::unique_ptr<Texture>> _framebufferTextures;
    std::vector<std::unique_ptr<Framebuffer>> _framebuffers;
    std::unique_ptr<GraphicsPipeline> _graphicsPipeline;
    std::unique_ptr<GraphicsPipeline> _graphicsPipelineSkybox;

    std::shared_ptr<Renderpass> _shadowRenderPass;
    std::unique_ptr<Framebuffer> _shadowFramebuffer;
    std::shared_ptr<Texture2DShadow> _shadowMap;
    std::unique_ptr<GraphicsPipeline> _shadowPipeline;

    std::unique_ptr<VertexBuffer> _vertexBufferCube;
    std::unique_ptr<IndexBuffer> _indexBufferCube;

    std::vector<Object> objects;
    UniformBufferCamera _ubCamera;
    UniformBufferObject _ubObject;
    UniformBufferLight _ubLight;

    std::unique_ptr<UniformBufferDynamic<UniformBufferObject>> _uniformBuffersObjects;
    std::unique_ptr<UniformBufferStruct<UniformBufferLight>> _uniformBuffersLight;
    std::unique_ptr<UniformBufferDynamic<UniformBufferCamera>> _dynamicUniformBuffersCamera;
    std::unique_ptr<UniformBufferTexture> _skyboxTextureUniform;
    std::unique_ptr<UniformBufferTexture> _shadowTextureUniform;

    std::shared_ptr<DescriptorPool> _descriptorPool;
    std::shared_ptr<DescriptorPool> _descriptorPoolSkybox;
    std::shared_ptr<DescriptorPool> _descriptorPoolShadow;

    std::unique_ptr<ShadowShaderProgram> _shadowShaderProgram;
    std::unique_ptr<PBRTesselationShaderProgram> _pbrShaderProgram;
    std::unique_ptr<SkyboxShaderProgram> _skyboxShaderProgram;

    std::unique_ptr<TextureCubemap> _textureCubemap;

    std::unique_ptr<DescriptorSet> _descriptorSetSkybox;
    std::unique_ptr<DescriptorSet> _descriptorSetShadow;
    std::unique_ptr<Screenshot> _screenshot;

    std::unique_ptr<CallbackManager> _callbackManager;
    std::unique_ptr<FPSCamera> _camera;

    std::vector<std::unique_ptr<CommandPool>> _commandPool;
    std::vector<std::unique_ptr<CommandBuffer>> _primaryCommandBuffer;
    std::vector<std::vector<std::unique_ptr<CommandBuffer>>> _commandBuffers;
    std::vector<std::vector<std::unique_ptr<CommandBuffer>>> _shadowCommandBuffers;

    std::unique_ptr<ThreadPool> _threadPool;
    std::vector<VkSemaphore> _shadowMapSemaphores;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame = 0;
    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 3;
    static constexpr uint32_t MAX_THREADS_IN_POOL = 2;

public:
    SingleApp();
    ~SingleApp();
    static SingleApp& getInstance();

    SingleApp(const SingleApp&) = delete;
    SingleApp(SingleApp&&) = delete;
    void operator=(const SingleApp&) = delete;

    void run() override;
private:
    void draw();
    VkFormat findDepthFormat() const;
    void createCommandBuffers();
    void createSyncObjects();
    void updateUniformBuffer(uint32_t currentImage);
    void recordCommandBuffer(VkCommandBuffer primaryCommandBuffer, uint32_t imageIndex);
    void recordOctreeSecondaryCommandBuffer(const VkCommandBuffer commandBuffer, const OctreeNode* node, const std::array<glm::vec4, NUM_CUBE_FACES>& planes);
    void recordShadowCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recreateSwapChain();

    void createDescriptorSets();
    void createPresentResources();
    void createShadowResources();

    void loadObjects();
};
