#pragma once

#include "application_base.h"

#include <render_pass/render_pass.h>
#include <command_buffer/command_buffer.h>
#include <render_pass/framebuffer/framebuffer.h>
#include <memory_objects/vertex_buffer.h>
#include <memory_objects/index_buffer.h>
#include <memory_objects/uniform_buffer.h>
#include <pipeline/graphics_pipeline.h>
#include <memory_objects/texture/texture_2D.h>
#include <model_loader/obj_loader/obj_loader.h>
#include <descriptor_set/descriptor_set.h>
#include <camera/fps_camera.h>
#include <window/callback_manager/fps_callback_manager.h>
#include <memory_objects/texture/texture_2D_depth.h>
#include <memory_objects/texture/texture_2D_color.h>
#include <memory_objects/texture/texture_2D_sampler.h>
#include <screenshot/screenshot.h>

class DoubleScreenshotApplication : public ApplicationBase {
    std::shared_ptr<Renderpass> _renderPass;
    std::unique_ptr<Framebuffer> _framebuffer;
    std::unique_ptr<VertexBuffer<Vertex>> _vertexBuffer;
    std::unique_ptr<IndexBuffer<uint16_t>> _indexBuffer;
    VertexData<Vertex, uint16_t> _vertexData;
    std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>> _uniformBuffers;
    std::unique_ptr<Pipeline> _graphicsPipeline;
    std::shared_ptr<Texture2DSampler> _texture;
    std::unique_ptr<DescriptorSets> _descriptorSets;
    std::unique_ptr<Screenshot> _screenshot;

    TinyOBJLoaderVertex _OBJLoader;
    std::unique_ptr<CallbackManager> _callbackManager;
    std::unique_ptr<FPSCamera> _camera;

    std::vector<VkCommandBuffer> _commandBuffers;

    std::vector<VkSemaphore> _imageAvailableSemaphores;
    std::vector<VkSemaphore> _renderFinishedSemaphores;
    std::vector<VkFence> _inFlightFences;

    uint32_t _currentFrame              = 0;
    const uint32_t MAX_FRAMES_IN_FLIGHT = 3;

	DoubleScreenshotApplication();
    ~DoubleScreenshotApplication();
public:
    static DoubleScreenshotApplication& getInstance();

    DoubleScreenshotApplication(const DoubleScreenshotApplication&) = delete;
    DoubleScreenshotApplication(DoubleScreenshotApplication&&) = delete;
    void operator=(const DoubleScreenshotApplication&) = delete;

    void run() override;
private:
    void draw();
    VkFormat findDepthFormat() const;
    void createSyncObjects();
    void updateUniformBuffer(uint32_t currentImage);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void recreateSwapChain();
};
