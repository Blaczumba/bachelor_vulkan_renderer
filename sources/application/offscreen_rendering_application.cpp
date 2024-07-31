#include "offscreen_rendering_application.h"

#include <algorithm>
#include <array>

OffscreenRendering::OffscreenRendering()
    : ApplicationBase() {

    createDescriptorSets();
    createPresentResources();
    createOffscreenResources();
    createShadowResources();
    
    _vertexData = TinyOBJLoaderVertex::extract<VertexPTN, uint16_t>(MODELS_PATH "cube.obj");
    _vertexBuffer = std::make_unique<VertexBuffer<VertexPTN>>(_logicalDevice, _vertexData.vertices);
    _indexBuffer = std::make_unique<IndexBuffer<uint16_t>>(_logicalDevice, _vertexData.indices);

    _vertexDataCube = TinyOBJLoaderVertex::extract<VertexP, uint16_t>(MODELS_PATH "cube.obj");
    _vertexBufferCube = std::make_unique<VertexBuffer<VertexP>>(_logicalDevice, _vertexDataCube.vertices);
    _indexBufferCube = std::make_unique<IndexBuffer<uint16_t>>(_logicalDevice, _vertexDataCube.indices);

    _commandBuffers = _logicalDevice->createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
    _offscreenCommandBuffers = _logicalDevice->createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
    _shadowCommandBuffers = _logicalDevice->createCommandBuffers(MAX_FRAMES_IN_FLIGHT);

    _screenshot = std::make_unique<Screenshot>(_logicalDevice);
    _screenshot->addImageToObserved(_framebuffer->getColorTextures()[0].getImage(), "hig_res_screenshot.ppm");
    _screenshot->addImageToObserved(_lowResTextureColorAttachment->getImage(), "low_res_screenshot.ppm");

    _camera = std::make_unique<FPSCamera>(glm::radians(45.0f), 1920.0f / 1080.0f, 0.01f, 100.0f);

    _callbackManager = std::make_unique<FPSCallbackManager>(std::dynamic_pointer_cast<WindowGLFW>(_window));
    _callbackManager->attach(_camera.get());
    _callbackManager->attach(_screenshot.get());

    createSyncObjects();
}

void OffscreenRendering::createDescriptorSets() {
    _texture = std::make_shared<Texture2DImage>(_logicalDevice, TEXTURES_PATH "drakan.jpg", VK_FORMAT_R8G8B8A8_SRGB, _physicalDevice->getMaxSamplerAnisotropy());
    _textureCubemap = std::make_shared<TextureCubemap>(_logicalDevice, TEXTURES_PATH "cubemap_yokohama_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, _physicalDevice->getMaxSamplerAnisotropy());
    _shadowMap = std::make_shared<Texture2DShadow>(_logicalDevice, 1024 * 2, 1024 * 2, VK_FORMAT_D32_SFLOAT);

    _uniformBuffersObjects = std::make_shared<UniformBufferDynamic<UniformBufferObject>>(_logicalDevice, 2);
    _uniformBuffersLight = std::make_shared<UniformBufferStruct<UniformBufferLight>>(_logicalDevice);
    _dynamicUniformBuffersCamera = std::make_shared<UniformBufferStruct<UniformBufferCamera>>(_logicalDevice);

    _textureUniform = std::make_shared<UniformBufferTexture>(_logicalDevice, _texture);
    _skyboxTextureUniform = std::make_shared<UniformBufferTexture>(_logicalDevice, _textureCubemap);
    _shadowTextureUniform = std::make_shared<UniformBufferTexture>(_logicalDevice, _shadowMap);

    _descriptorSetLayout = std::make_shared<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayout->create();

    _descriptorSetLayoutSkybox = std::make_shared<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayoutSkybox->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayoutSkybox->addLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
    _descriptorSetLayoutSkybox->create();

    _descriptorSetLayoutShadow = std::make_shared<DescriptorSetLayout>(_logicalDevice);
    _descriptorSetLayoutShadow->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayoutShadow->addLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
    _descriptorSetLayoutShadow->create();

    _descriptorPool = std::make_shared<DescriptorPool>(_logicalDevice, _descriptorSetLayout, 5);
    _descriptorPoolSkybox = std::make_shared<DescriptorPool>(_logicalDevice, _descriptorSetLayoutSkybox, 5);
    _descriptorPoolShadow = std::make_shared<DescriptorPool>(_logicalDevice, _descriptorSetLayoutShadow, 5);

    _descriptorSet = _descriptorPool->createDesriptorSet();
    _descriptorSetSkybox = _descriptorPoolSkybox->createDesriptorSet();
    _descriptorSetShadow = _descriptorPoolShadow->createDesriptorSet();

    _descriptorSet->updateDescriptorSet({ _dynamicUniformBuffersCamera, _textureUniform, _uniformBuffersLight, _uniformBuffersObjects, _shadowTextureUniform }, 2);
    _descriptorSetSkybox->updateDescriptorSet({ _dynamicUniformBuffersCamera, _skyboxTextureUniform });
    _descriptorSetShadow->updateDescriptorSet({ _uniformBuffersLight,  _uniformBuffersObjects }, 2);

    _pushConstants = std::make_unique<PushConstants>(*_physicalDevice);
    _pushConstants->addPushConstant<UniformBufferObject>(VK_SHADER_STAGE_VERTEX_BIT);

    // Initializing static uniform buffers as they do not need to be updated each frame.
    _ubObject.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, 0.0f));
    _uniformBuffersObjects->updateUniformBuffer(&_ubObject, 0);
    _ubObject.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -3.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 0.1f, 10.0f));
    _uniformBuffersObjects->updateUniformBuffer(&_ubObject, 1);
    _uniformBuffersObjects->makeUpdatesVisible();

    _ubLight.pos = glm::vec3(2.0f, 2.0f, 2.0f);
    _ubLight.projView = glm::perspective(glm::radians(45.0f), 1.0f, 0.01f, 12.0f);
    _ubLight.projView[1][1] = -_ubLight.projView[1][1];
    _ubLight.projView = _ubLight.projView * glm::lookAt(_ubLight.pos, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    _uniformBuffersLight->updateUniformBuffer(&_ubLight);
}

void OffscreenRendering::createPresentResources() {
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;
    VkFormat swapchainImageFormat = _swapchain->getVkFormat();

    AttachmentLayout attachmentsLayout;
    attachmentsLayout.addAttachment(ColorResolvePresentAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE));
    attachmentsLayout.addAttachment(ColorResolveAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE));
    attachmentsLayout.addAttachment(ColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));
    attachmentsLayout.addAttachment(ColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));
    attachmentsLayout.addAttachment(DepthAttachment(findDepthFormat(), VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));

    Subpass subpass(attachmentsLayout);
    subpass.addSubpassOutputAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.addSubpassOutputAttachment(1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.addSubpassOutputAttachment(2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.addSubpassOutputAttachment(3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.addSubpassOutputAttachment(4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    _renderPass = std::make_shared<Renderpass>(_logicalDevice, attachmentsLayout);
    _renderPass->addSubpass(subpass);
    _renderPass->addDependency(VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    );
    _renderPass->create();

    _framebuffer = std::make_unique<Framebuffer>(_logicalDevice, _swapchain, _renderPass);
    _graphicsPipeline = std::make_unique<GraphicsPipeline<VertexPTN>>(_logicalDevice, _renderPass, _descriptorSet->getVkDescriptorSetLayout(), *_pushConstants, msaaSamples, "vert.spv", "frag.spv", true, 0.0f, 0.0f);
    _graphicsPipelineSkybox = std::make_unique<GraphicsPipeline<VertexP>>(_logicalDevice, _renderPass, _descriptorSetSkybox->getVkDescriptorSetLayout(), *_pushConstants, msaaSamples, "skybox.vert.spv", "skybox.frag.spv", false, 0.0f, 0.0f);
}

void OffscreenRendering::createOffscreenResources() {
    VkSampleCountFlagBits lowResMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkFormat swapchainImageFormat = _swapchain->getVkFormat();
    VkExtent2D extent = _swapchain->getExtent();
    VkExtent2D lowResExtent = { extent.width / 4, extent.height / 4 };

    AttachmentLayout attachmentLayout;
    attachmentLayout.addAttachment(ColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE, lowResMsaaSamples));
    attachmentLayout.addAttachment(DepthAttachment(findDepthFormat(), VK_ATTACHMENT_STORE_OP_DONT_CARE, lowResMsaaSamples));

    Subpass subpass(attachmentLayout);
    subpass.addSubpassOutputAttachment(0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    subpass.addSubpassOutputAttachment(1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    _lowResRenderPass = std::make_shared<Renderpass>(_logicalDevice, attachmentLayout);
    _lowResRenderPass->addSubpass(subpass);
    _lowResRenderPass->create();

    _lowResTextureColorAttachment = std::make_shared<Texture2DColor>(_logicalDevice, swapchainImageFormat, lowResMsaaSamples, lowResExtent);
    _lowResTextureDepthAttachment = std::make_shared<Texture2DDepth>(_logicalDevice, findDepthFormat(), lowResMsaaSamples, lowResExtent);

    // auto lowResTextureColorResolveView = _lowResTextureColorResolveAttachment->getImage().view;
    auto lowResTextureColorView = _lowResTextureColorAttachment->getImage().view;
    auto lowResTextureDepthView = _lowResTextureDepthAttachment->getImage().view;
    std::vector<std::vector<VkImageView>> lowResViews = {
        {/* lowResTextureColorResolveView,*/ lowResTextureColorView, lowResTextureDepthView},
        {/* lowResTextureColorResolveView,*/ lowResTextureColorView, lowResTextureDepthView},
        {/* lowResTextureColorResolveView,*/ lowResTextureColorView, lowResTextureDepthView},
    };

    _lowResFramebuffer = std::make_unique<Framebuffer>(_logicalDevice, std::move(lowResViews), _lowResRenderPass, lowResExtent, MAX_FRAMES_IN_FLIGHT);
    _lowResGraphicsPipeline = std::make_unique<GraphicsPipeline<VertexPTN>>(_logicalDevice, _lowResRenderPass, _descriptorSet->getVkDescriptorSetLayout(), *_pushConstants, lowResMsaaSamples, "off.vert.spv", "off.frag.spv", true, 0.0f, 0.0f);
    _lowResGraphicsPipelineSkybox = std::make_unique<GraphicsPipeline<VertexP>>(_logicalDevice, _lowResRenderPass, _descriptorSetSkybox->getVkDescriptorSetLayout(), *_pushConstants, lowResMsaaSamples, "skybox_offscreen.vert.spv", "skybox_offscreen.frag.spv", false, 0.0f, 0.0f);
}

void OffscreenRendering::createShadowResources() {
    VkSampleCountFlagBits shadowMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkFormat imageFormat = VK_FORMAT_D32_SFLOAT;
    uint32_t width = 1024 * 2;
    uint32_t height = 1024 * 2;
    VkExtent2D extent = { width, height };

    AttachmentLayout attachmentLayout;
    attachmentLayout.addAttachment(ShadowAttachment(imageFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    Subpass subpass(attachmentLayout);
    subpass.addSubpassOutputAttachment(0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    _shadowRenderPass = std::make_shared<Renderpass>(_logicalDevice, attachmentLayout);
    _shadowRenderPass->addSubpass(subpass);
    _shadowRenderPass->create();

    std::vector<std::vector<VkImageView>> shadowViews = {
        {_shadowMap->getImage().view},
        {_shadowMap->getImage().view},
        {_shadowMap->getImage().view},
    };

    _shadowFramebuffer = std::make_unique<Framebuffer>(_logicalDevice, std::move(shadowViews), _shadowRenderPass, extent, MAX_FRAMES_IN_FLIGHT);
    _shadowPipeline = std::make_unique<GraphicsPipeline<VertexPTN>>(_logicalDevice, _shadowRenderPass, _descriptorSetShadow->getVkDescriptorSetLayout(), *_pushConstants, shadowMsaaSamples, "shadow.vert.spv", "shadow.frag.spv", true, 0.7f, 2.0f);
}

OffscreenRendering::~OffscreenRendering() {
    VkDevice device = _logicalDevice->getVkDevice();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, _inFlightFences[i], nullptr);
    }
}

OffscreenRendering& OffscreenRendering::getInstance() {
    static OffscreenRendering application;
    return application;
}

void OffscreenRendering::run() {
    updateUniformBuffer(_currentFrame);
    {
        SingleTimeCommandBuffer handle(*_logicalDevice);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        recordShadowCommandBuffer(commandBuffer, 0);
    }

    while (_window->open()) {
        _callbackManager->pollEvents();
        draw();
    }
    vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

void OffscreenRendering::draw() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkWaitForFences(device, 1, &_inFlightFences[_currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex;

    VkResult result = _swapchain->acquireNextImage(_imageAvailableSemaphores[_currentFrame], &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    updateUniformBuffer(_currentFrame);

    vkResetFences(device, 1, &_inFlightFences[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    vkResetCommandBuffer(_offscreenCommandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    //vkResetCommandBuffer(_shadowCommandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    //recordShadowCommandBuffer(_shadowCommandBuffers[_currentFrame], imageIndex);
    recordOffscreenCommandBuffer(_offscreenCommandBuffers[_currentFrame], imageIndex);
    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::array<VkCommandBuffer, 2> submitCommands = { _commandBuffers[_currentFrame], _offscreenCommandBuffers[_currentFrame] };
    submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommands.size());
    submitInfo.pCommandBuffers = submitCommands.data();

    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_logicalDevice->graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    result = _swapchain->present(imageIndex, signalSemaphores[0]);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    if (++_currentFrame == MAX_FRAMES_IN_FLIGHT)
        _currentFrame = 0;

}

VkFormat OffscreenRendering::findDepthFormat() const {
    std::array<VkFormat, 3> depthFormats = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
    };

    auto formatPtr = std::find_if(depthFormats.cbegin(), depthFormats.cend(), [&](VkFormat format) {
        return _physicalDevice->checkTextureFormatSupport(format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
        });

    if (formatPtr == depthFormats.cend()) {
        throw std::runtime_error("failed to find depth format!");
    }

    return *formatPtr;
}

void OffscreenRendering::createSyncObjects() {
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkDevice device = _logicalDevice->getVkDevice();

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void OffscreenRendering::updateUniformBuffer(uint32_t currentFrame) {
    const auto& swapchainExtent = _swapchain->getExtent();

    _ubCamera.view = _camera->getViewMatrix();
    _ubCamera.proj = _camera->getProjectionMatrix();
    _ubCamera.pos  = _camera->getPosition();

    _dynamicUniformBuffersCamera->updateUniformBuffer(&_ubCamera);
}

void OffscreenRendering::recordOffscreenCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _lowResRenderPass->getVkRenderPass();
    renderPassInfo.framebuffer = _lowResFramebuffer->getVkFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    VkExtent2D extent = { _lowResTextureColorAttachment->getImage().extent.width, _lowResTextureColorAttachment->getImage().extent.height };
    renderPassInfo.renderArea.extent = extent;

    const auto& clearValues = _lowResRenderPass->getClearValues();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize offsets[] = { 0 };

    // OBJECT
    vkCmdBindPipeline(commandBuffer, _lowResGraphicsPipeline->getVkPipelineBindPoint(), _lowResGraphicsPipeline->getVkPipeline());

    VkBuffer vertexBuffers[] = { _vertexBuffer->getVkBuffer() };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getVkBuffer(), 0, _indexBuffer->getIndexType());

    _descriptorSet->bindDescriptorSet(commandBuffer, *_lowResGraphicsPipeline, 0);
 
    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    _descriptorSet->bindDescriptorSet(commandBuffer, *_lowResGraphicsPipeline, 1);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    // SKYBOX
    vkCmdBindPipeline(commandBuffer, _lowResGraphicsPipelineSkybox->getVkPipelineBindPoint(), _lowResGraphicsPipelineSkybox->getVkPipeline());

    VkBuffer vertexBuffersCube[] = { _vertexBufferCube->getVkBuffer() };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersCube, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBufferCube->getVkBuffer(), 0, _indexBufferCube->getIndexType());

    _descriptorSetSkybox->bindDescriptorSet(commandBuffer, *_lowResGraphicsPipelineSkybox);

    vkCmdDrawIndexed(commandBuffer, _indexBufferCube->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void OffscreenRendering::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass->getVkRenderPass();
    renderPassInfo.framebuffer = _framebuffer->getVkFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = _swapchain->getExtent();

    const auto& clearValues = _renderPass->getClearValues();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    const VkExtent2D& swapchainExtent = _swapchain->getExtent();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize offsets[] = { 0 };

    // OBJECT
    vkCmdBindPipeline(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(), _graphicsPipeline->getVkPipeline());

    VkBuffer vertexBuffers[] = { _vertexBuffer->getVkBuffer() };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getVkBuffer(), 0, _indexBuffer->getIndexType());

    _descriptorSet->bindDescriptorSet(commandBuffer, *_graphicsPipeline, 0);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    _descriptorSet->bindDescriptorSet(commandBuffer, *_graphicsPipeline, 1);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    // SKYBOX
    vkCmdBindPipeline(commandBuffer, _graphicsPipelineSkybox->getVkPipelineBindPoint(), _graphicsPipelineSkybox->getVkPipeline());

    VkBuffer vertexBuffersCube[] = { _vertexBufferCube->getVkBuffer() };
    
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersCube, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBufferCube->getVkBuffer(), 0, _indexBufferCube->getIndexType());

    _descriptorSetSkybox->bindDescriptorSet(commandBuffer, *_graphicsPipelineSkybox);

    vkCmdDrawIndexed(commandBuffer, _indexBufferCube->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void OffscreenRendering::recordShadowCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to begin recording command buffer!");
    //}

    VkExtent2D extent = { _shadowMap->getImage().extent.width, _shadowMap->getImage().extent.height };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _shadowRenderPass->getVkRenderPass();
    renderPassInfo.framebuffer = _shadowFramebuffer->getVkFramebuffers()[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    const auto& clearValues = _shadowRenderPass->getClearValues();
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize offsets[] = { 0 };

    // OBJECT
    vkCmdBindPipeline(commandBuffer, _shadowPipeline->getVkPipelineBindPoint(), _shadowPipeline->getVkPipeline());

    VkBuffer vertexBuffers[] = { _vertexBuffer->getVkBuffer() };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getVkBuffer(), 0, _indexBuffer->getIndexType());

    _descriptorSetShadow->bindDescriptorSet(commandBuffer, *_shadowPipeline, 0);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    _descriptorSetShadow->bindDescriptorSet(commandBuffer, *_shadowPipeline, 1);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    //if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to record command buffer!");
    //}
}

void OffscreenRendering::recreateSwapChain() {
    VkExtent2D extent{};
    while (extent.width == 0 || extent.height == 0) {
        extent = _window->getFramebufferSize();
        glfwWaitEvents();
    }

    _camera->setAspectRatio(static_cast<float>(extent.width) / extent.height);

    vkDeviceWaitIdle(_logicalDevice->getVkDevice());

    _swapchain->recrete();

    _framebuffer = std::make_unique<Framebuffer>(_logicalDevice, _swapchain, _renderPass);
}