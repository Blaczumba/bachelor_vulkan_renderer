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
#include <memory_objects/texture/texture_cubemap.h>
#include <memory_objects/uniform_buffer/push_constants.h>
#include <screenshot/screenshot.h>

class OffscreenRendering : public ApplicationBase {
    std::shared_ptr<Renderpass> _renderPass;
    std::unique_ptr<Framebuffer> _framebuffer;
    std::unique_ptr<Pipeline> _graphicsPipeline;
    std::unique_ptr<Pipeline> _graphicsPipelineSkybox;

    std::shared_ptr<Renderpass> _lowResRenderPass;
    std::unique_ptr<Framebuffer> _lowResFramebuffer;
    // std::shared_ptr<Texture2DColor> _lowResTextureColorResolveAttachment;
    std::shared_ptr<Texture2DColor> _lowResTextureColorAttachment;
    std::shared_ptr<Texture2DDepth> _lowResTextureDepthAttachment;
    std::unique_ptr<Pipeline> _lowResGraphicsPipeline;
    std::unique_ptr<Pipeline> _lowResGraphicsPipelineSkybox;

    std::unique_ptr<VertexBuffer<VertexPT>> _vertexBuffer;
    std::unique_ptr<IndexBuffer<uint16_t>> _indexBuffer;
    VertexData<VertexPT, uint16_t> _vertexData;

    std::unique_ptr<VertexBuffer<VertexP>> _vertexBufferCube;
    std::unique_ptr<IndexBuffer<uint16_t>> _indexBufferCube;
    VertexData<VertexP, uint16_t> _vertexDataCube;

    std::vector<std::shared_ptr<UniformBufferStruct<UniformBufferObject>>> _mvpUnuiformBuffers;
    std::shared_ptr<UniformBufferDynamic<UniformBufferObject>> _dynammicUniformBuffers;
    std::shared_ptr<UniformBufferTexture> _textureUniform;
    std::shared_ptr<UniformBufferTexture> _skyboxTextureUniform;
    std::unique_ptr<PushConstants> _pushConstants;

    std::shared_ptr<DescriptorSetLayout> _descriptorSetLayout;
    std::shared_ptr<DescriptorSetLayout> _descriptorSetLayoutSkybox;
    std::shared_ptr<DescriptorPool> _descriptorPool;
    std::shared_ptr<DescriptorPool> _descriptorPoolSkybox;
    UniformBufferObject _ubo;

    std::shared_ptr<Texture2DImage> _texture;
    std::shared_ptr<TextureCubemap> _textureCubemap;

    std::unique_ptr<DescriptorSet> _descriptorSet;
    std::unique_ptr<DescriptorSet> _descriptorSetSkybox;
    std::unique_ptr<Screenshot> _screenshot;

    std::unique_ptr<CallbackManager> _callbackManager;
    std::unique_ptr<FPSCamera> _camera;

    std::vector<VkCommandBuffer> _commandBuffers;
    std::vector<VkCommandBuffer> _offscreenCommandBuffers;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame              = 0;
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
    void recreateSwapChain();

    void createDescriptorSets();
    void createOffscreenResources();
    void createPresentResources();
    void createSkyboxResources();
};
