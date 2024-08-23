#pragma once

#include "application_base.h"

#include <render_pass/render_pass.h>
#include <command_buffer/command_buffer.h>
#include <render_pass/framebuffer/framebuffer.h>
#include <memory_objects/vertex_buffer.h>
#include <memory_objects/index_buffer.h>
#include <memory_objects/uniform_buffer/uniform_buffer.h>
#include <pipeline/graphics_pipeline.h>
#include <memory_objects/texture/texture_2D.h>
#include <model_loader/obj_loader/obj_loader.h>
#include <descriptor_set/descriptor_set.h>
#include <camera/fps_camera.h>
#include <window/callback_manager/fps_callback_manager.h>
#include <memory_objects/texture/texture_2D_depth.h>
#include <memory_objects/texture/texture_2D_color.h>
#include <memory_objects/texture/texture_2D_image.h>
#include <memory_objects/texture/texture_2D_shadow.h>
#include <memory_objects/texture/texture_cubemap.h>
#include <memory_objects/uniform_buffer/push_constants.h>
#include <descriptor_set/descriptor_set_layout.h>
#include <descriptor_set/descriptor_pool.h>
#include <screenshot/screenshot.h>

#include <unordered_map>

struct Object {
    std::unique_ptr<VertexBuffer<VertexPTNTB>> vertexBufferPTNTB;
    std::unique_ptr<VertexBuffer<VertexP>> vertexBufferP;
    std::unique_ptr<IndexBuffer> indexBuffer;

    uint32_t dynamicUniformIndex;

    std::unique_ptr<DescriptorSet> _descriptorSet;

    glm::mat4 model;
};

class OffscreenRendering : public ApplicationBase {
    std::vector<VertexData<VertexPTNTB, uint32_t>> _newVertexDataTBN;
    std::vector<std::unique_ptr<Texture2DImage>> _textures;
    std::unordered_map<std::string, std::unique_ptr<UniformBufferTexture>> _uniformMap;
    std::unordered_map<std::string, std::unique_ptr<VertexBuffer<VertexPTNTB>>> _vertexBufferMap;
    std::unordered_map<std::string, std::unique_ptr<IndexBuffer>> _indexBufferMap;
    std::vector<Object> _objects;

    std::shared_ptr<Renderpass> _renderPass;
    std::unique_ptr<Framebuffer> _framebuffer;
    std::unique_ptr<GraphicsPipeline> _graphicsPipeline;
    std::unique_ptr<GraphicsPipeline> _graphicsPipelineSkybox;

    std::shared_ptr<Renderpass> _lowResRenderPass;
    std::unique_ptr<Framebuffer> _lowResFramebuffer;
    std::unique_ptr<Texture2DColor> _lowResTextureColorAttachment;
    std::unique_ptr<Texture2DDepth> _lowResTextureDepthAttachment;
    std::unique_ptr<GraphicsPipeline> _lowResGraphicsPipeline;
    std::unique_ptr<GraphicsPipeline> _lowResGraphicsPipelineSkybox;

    std::shared_ptr<Renderpass> _shadowRenderPass;
    std::unique_ptr<Framebuffer> _shadowFramebuffer;
    std::unique_ptr<Texture2DShadow> _shadowMap;
    std::unique_ptr<GraphicsPipeline> _shadowPipeline;

    std::unique_ptr<VertexBuffer<VertexP>> _vertexBufferCube;
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
    std::unique_ptr<PBRShaderProgram> _pbrShaderProgram;
    std::unique_ptr<PBRShaderOffscreenProgram> _pbrOffscreenShaderProgram;
    std::unique_ptr<SkyboxShaderProgram> _skyboxShaderProgram;
    std::unique_ptr<SkyboxOffscreenShaderProgram> _skyboxOffscreenShaderProgram;

    std::unique_ptr<TextureCubemap> _textureCubemap;

    std::unique_ptr<DescriptorSet> _descriptorSetSkybox;
    std::unique_ptr<DescriptorSet> _descriptorSetShadow;
    std::unique_ptr<Screenshot> _screenshot;

    std::unique_ptr<CallbackManager> _callbackManager;
    std::unique_ptr<FPSCamera> _camera;

    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkCommandBuffer> _offscreenCommandBuffers;
    std::vector<VkCommandBuffer> _shadowCommandBuffers;

    std::vector<VkSemaphore> _shadowMapSemaphores;
    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame = 0;
    const uint32_t MAX_FRAMES_IN_FLIGHT = 3;

    OffscreenRendering();
    ~OffscreenRendering();
public:
    static OffscreenRendering& getInstance();

    OffscreenRendering(const OffscreenRendering&) = delete;
    OffscreenRendering(OffscreenRendering&&) = delete;
    void operator=(const OffscreenRendering&) = delete;

    void run() override;
private:
    void draw();
    VkFormat findDepthFormat() const;
    void createSyncObjects();
    void updateUniformBuffer(uint32_t currentImage);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordOffscreenCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recordShadowCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recreateSwapChain();

    void createDescriptorSets();
    void createOffscreenResources();
    void createPresentResources();
    void createShadowResources();

    void loadObjects();
};
