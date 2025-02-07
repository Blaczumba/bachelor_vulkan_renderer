#include "offscreen_rendering_application.h"

#include "entity_component_system/system/movement_system.h"
#include "model_loader/tiny_gltf_loader/tiny_gltf_loader.h"
#include "thread_pool/thread_pool.h"
#include "utils/utils.h"

#include <algorithm>
#include <any>
#include <array>
#include <chrono>

OffscreenRendering::OffscreenRendering()
    : ApplicationBase() {
    TextureFactory factory;
    auto tx = factory.createTexture<Texture2DImage>(*_logicalDevice, TEXTURES_PATH "brickwall.jpg", VK_FORMAT_R8G8B8A8_SRGB, 16.0f);
    _newVertexDataTBN = LoadGLTF<VertexPTNT, uint16_t>(MODELS_PATH "sponza/scene.gltf");
    
    createDescriptorSets();
    loadObjects();
    createPresentResources();
    createOffscreenResources();
    createShadowResources();

    VertexData<VertexP, uint16_t> vertexDataCube = TinyOBJLoaderVertex::extract<VertexP, uint16_t>(MODELS_PATH "cube.obj");
    _vertexBufferCube = std::make_unique<VertexBuffer>(*_logicalDevice, vertexDataCube.vertices);
    _indexBufferCube = std::make_unique<IndexBuffer>(*_logicalDevice, vertexDataCube.indices);

    createCommandBuffers();

    _screenshot = std::make_unique<Screenshot>(*_logicalDevice);
    _screenshot->addImageToObserved(_framebufferTextures[1]->getImage(), "hig_res_screenshot.ppm");
    _screenshot->addImageToObserved(_lowResFramebufferTextures[0]->getImage(), "low_res_screenshot.ppm");

    _camera = std::make_unique<FPSCamera>(glm::radians(45.0f), 1920.0f / 1080.0f, 0.01f, 100.0f);

    _callbackManager = std::make_unique<FPSCallbackManager>(std::dynamic_pointer_cast<WindowGLFW>(_window));
    _callbackManager->attach(_camera.get());
    _callbackManager->attach(_screenshot.get());

    createSyncObjects();
}

void OffscreenRendering::loadObjects() {
    const auto& propertyManager = _physicalDevice->getPropertyManager();
    float maxSamplerAnisotropy = propertyManager.getMaxSamplerAnisotropy();
    uint32_t index = 0;

    for (uint32_t i = 0; i < _newVertexDataTBN.size(); i++) {
        if (_newVertexDataTBN[i].normalTextures.empty() || _newVertexDataTBN[i].metallicRoughnessTextures.empty())
            continue;
        const std::string diffusePath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].diffuseTextures[0];
        const std::string metallicRoughnessPath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].metallicRoughnessTextures[0];
        const std::string normalPath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].normalTextures[0];

        if (_uniformMap.find(diffusePath) == _uniformMap.cend()) {
            _textures.emplace_back(std::make_unique<Texture2DImage>(*_logicalDevice, diffusePath, VK_FORMAT_R8G8B8A8_SRGB, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(diffusePath, std::make_unique<UniformBufferTexture>(*_textures.back())));
        }
        if (_uniformMap.find(normalPath) == _uniformMap.cend()) {
            _textures.emplace_back(std::make_unique<Texture2DImage>(*_logicalDevice, normalPath, VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(normalPath, std::make_unique<UniformBufferTexture>(*_textures.back())));
        }
        if (_uniformMap.find(metallicRoughnessPath) == _uniformMap.cend()) {
            _textures.emplace_back(std::make_unique<Texture2DImage>(*_logicalDevice, metallicRoughnessPath, VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(metallicRoughnessPath, std::make_unique<UniformBufferTexture>(*_textures.back())));
        }

        auto descriptorSet = _descriptorPool->createDesriptorSet();
        descriptorSet->updateDescriptorSet({ _dynamicUniformBuffersCamera.get(), _uniformMap[diffusePath].get(), _uniformBuffersLight.get(), _uniformBuffersObjects.get(), _shadowTextureUniform.get(), _uniformMap[normalPath].get(), _uniformMap[metallicRoughnessPath].get() });

        std::vector<glm::vec3> pVertexData;
        std::transform(_newVertexDataTBN[i].vertices.cbegin(), _newVertexDataTBN[i].vertices.cend(), std::back_inserter(pVertexData), [](const VertexPTNT& vertex) { return vertex.pos; });
        _objects.push_back(Object{
            std::make_unique<VertexBuffer>(*_logicalDevice, _newVertexDataTBN[i].vertices),
            std::make_unique<VertexBuffer>(*_logicalDevice, pVertexData),
            std::make_unique<IndexBuffer>(*_logicalDevice, _newVertexDataTBN[i].indices),
            index,
            std::move(descriptorSet),
            _newVertexDataTBN[i].model,
            createAABBfromVertices(pVertexData, _newVertexDataTBN[i].model) }
        );

        _ubObject.model = _newVertexDataTBN[i].model;
        _uniformBuffersObjects->updateUniformBuffer(&_ubObject, index++);
    }
    // _uniformBuffersObjects->makeUpdatesVisible();

    AABB sceneAABB = _objects[0].volume;
    for (int i = 1; i < _objects.size(); i++) {
        sceneAABB.extend(_objects[i].volume);
    }
    _octree = std::make_unique<Octree>(sceneAABB);

    for (const auto& object : _objects)
        _octree->addObject(&object);
}

void OffscreenRendering::createDescriptorSets() {
    const auto& propertyManager = _physicalDevice->getPropertyManager();
    float maxSamplerAnisotropy = propertyManager.getMaxSamplerAnisotropy();
    _textureCubemap = std::make_unique<TextureCubemap>(*_logicalDevice, TEXTURES_PATH "cubemap_yokohama_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy);

    _shadowMap = std::make_shared<Texture2DShadow>(*_logicalDevice, 1024 * 2, 1024 * 2, VK_FORMAT_D32_SFLOAT);

    _uniformBuffersObjects = std::make_unique<UniformBufferDynamic<UniformBufferObject>>(*_logicalDevice, _newVertexDataTBN.size());
    _uniformBuffersLight = std::make_unique<UniformBufferStruct<UniformBufferLight>>(*_logicalDevice);
    _dynamicUniformBuffersCamera = std::make_unique<UniformBufferDynamic<UniformBufferCamera>>(*_logicalDevice, MAX_FRAMES_IN_FLIGHT);

    _skyboxTextureUniform = std::make_unique<UniformBufferTexture>(*_textureCubemap);
    _shadowTextureUniform = std::make_unique<UniformBufferTexture>(*_shadowMap);

    _pbrShaderProgram = std::make_unique<PBRShaderProgram>(*_logicalDevice);
    _pbrOffscreenShaderProgram = std::make_unique<PBRShaderOffscreenProgram>(*_logicalDevice);
    _skyboxShaderProgram = std::make_unique<SkyboxShaderProgram>(*_logicalDevice);
    _skyboxOffscreenShaderProgram = std::make_unique<SkyboxOffscreenShaderProgram>(*_logicalDevice);
    _shadowShaderProgram = std::make_unique<ShadowShaderProgram>(*_logicalDevice);

    _descriptorPool = std::make_shared<DescriptorPool>(*_logicalDevice, _pbrShaderProgram->getDescriptorSetLayout(), 150);
    _descriptorPoolSkybox = std::make_shared<DescriptorPool>(*_logicalDevice, _skyboxShaderProgram->getDescriptorSetLayout(), 1);
    _descriptorPoolShadow = std::make_shared<DescriptorPool>(*_logicalDevice, _shadowShaderProgram->getDescriptorSetLayout(), 1);

    _descriptorSetSkybox = _descriptorPoolSkybox->createDesriptorSet();
    _descriptorSetShadow = _descriptorPoolShadow->createDesriptorSet();

    _descriptorSetSkybox->updateDescriptorSet({ _dynamicUniformBuffersCamera.get(), _skyboxTextureUniform.get() });
    _descriptorSetShadow->updateDescriptorSet({ _uniformBuffersLight.get(),  _uniformBuffersObjects.get()});

    _ubLight.pos = glm::vec3(15.1891f, 2.66408f, -0.841221f);
    _ubLight.projView = glm::perspective(glm::radians(120.0f), 1.0f, 0.01f, 40.0f);
    _ubLight.projView[1][1] = -_ubLight.projView[1][1];
    _ubLight.projView = _ubLight.projView * glm::lookAt(_ubLight.pos, glm::vec3(-3.82383f, 3.66503f, 1.30751f), glm::vec3(0.0f, 1.0f, 0.0f));
    _uniformBuffersLight->updateUniformBuffer(&_ubLight);
}

void OffscreenRendering::createPresentResources() {
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;
    VkFormat swapchainImageFormat = _swapchain->getVkFormat();
    VkExtent2D extent = _swapchain->getExtent();

    AttachmentLayout attachmentsLayout;
    attachmentsLayout.addAttachment(ColorResolvePresentAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE));
    attachmentsLayout.addAttachment(ColorResolveAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE));
    attachmentsLayout.addAttachment(ColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));
    attachmentsLayout.addAttachment(ColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));
    attachmentsLayout.addAttachment(DepthAttachment(findDepthFormat(), VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples));

    Subpass subpass(attachmentsLayout);
    subpass.addSubpassOutputAttachment(0);
    subpass.addSubpassOutputAttachment(1);
    subpass.addSubpassOutputAttachment(2);
    subpass.addSubpassOutputAttachment(3);
    subpass.addSubpassOutputAttachment(4);

    _renderPass = std::make_shared<Renderpass>(*_logicalDevice, attachmentsLayout);
    _renderPass->addSubpass(subpass);
    _renderPass->addDependency(VK_SUBPASS_EXTERNAL,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
    );
    _renderPass->create();

    _framebufferTextures = createTexturesFromRenderpass(*_singleTimeCommandPool, *_renderPass, extent);
    const std::vector<VkImageView>& swapchainViews = _swapchain->getVkImageViews();
    size_t swapchainImagesCount = _swapchain->getImagesCount();
    _framebuffers.reserve(swapchainImagesCount);
    for (size_t i = 0; i < swapchainImagesCount; i++) {
        _framebuffers.emplace_back(std::make_unique<Framebuffer>(*_renderPass, extent, combineViewsForFramebuffer(_framebufferTextures, swapchainViews[i])));
    }

    GraphicsPipelineParameters parameters;
    parameters.msaaSamples = msaaSamples;
    _graphicsPipeline = std::make_unique<GraphicsPipeline>(*_renderPass);
    _graphicsPipeline->setShaderProgram(_pbrShaderProgram.get());
    _graphicsPipeline->setPipelineParameters(parameters);
    _graphicsPipeline->create();

    parameters.cullMode = VK_CULL_MODE_FRONT_BIT;
    _graphicsPipelineSkybox = std::make_unique<GraphicsPipeline>(*_renderPass);
    _graphicsPipelineSkybox->setShaderProgram(_skyboxShaderProgram.get());
    _graphicsPipelineSkybox->setPipelineParameters(parameters);
    _graphicsPipelineSkybox->create();
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
    subpass.addSubpassOutputAttachment(0);
    subpass.addSubpassOutputAttachment(1);

    _lowResRenderPass = std::make_shared<Renderpass>(*_logicalDevice, attachmentLayout);
    _lowResRenderPass->addSubpass(subpass);
    _lowResRenderPass->create();

    _lowResFramebufferTextures = createTexturesFromRenderpass(*_lowResRenderPass, lowResExtent);
    size_t swapchainImagesCount = _swapchain->getImagesCount();
    for (size_t i = 0; i < swapchainImagesCount; i++) {
        std::vector<VkImageView> imageViews;
        std::transform(_lowResFramebufferTextures.cbegin(), _lowResFramebufferTextures.cend(), std::back_inserter(imageViews), [this, i](const std::unique_ptr<Texture>& texture) { return texture->getImage().view; });
        _lowResFramebuffers.emplace_back(std::make_unique<Framebuffer>(*_lowResRenderPass, lowResExtent, imageViews));
    }

    GraphicsPipelineParameters parameters;
    parameters.msaaSamples = lowResMsaaSamples;
    _lowResGraphicsPipeline = std::make_unique<GraphicsPipeline>(*_lowResRenderPass);
    _lowResGraphicsPipeline->setShaderProgram(_pbrOffscreenShaderProgram.get());
    _lowResGraphicsPipeline->setPipelineParameters(parameters);
    _lowResGraphicsPipeline->create();

    parameters.cullMode = VK_CULL_MODE_FRONT_BIT;
    _lowResGraphicsPipelineSkybox = std::make_unique<GraphicsPipeline>(*_lowResRenderPass);
    _lowResGraphicsPipelineSkybox->setShaderProgram(_skyboxOffscreenShaderProgram.get());
    _lowResGraphicsPipelineSkybox->setPipelineParameters(parameters);
    _lowResGraphicsPipelineSkybox->create();
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
    subpass.addSubpassOutputAttachment(0);

    _shadowRenderPass = std::make_shared<Renderpass>(*_logicalDevice, attachmentLayout);
    _shadowRenderPass->addSubpass(subpass);
    _shadowRenderPass->create();

    std::vector<VkImageView> imageViews = { _shadowMap->getImage().view };
    _shadowFramebuffer = std::make_unique<Framebuffer>(*_shadowRenderPass, extent, imageViews);

    GraphicsPipelineParameters parameters;
    parameters.depthBiasConstantFactor = 0.7f;
    parameters.depthBiasSlopeFactor = 2.0f;
    _shadowPipeline = std::make_unique<GraphicsPipeline>(*_shadowRenderPass);
    _shadowPipeline->setShaderProgram(_shadowShaderProgram.get());
    _shadowPipeline->setPipelineParameters(parameters);
    _shadowPipeline->create();
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

    for (auto& object : _objects) {
        object.vertexBufferP = nullptr;
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

    //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    //    _commandBuffers[i]->resetCommandBuffer();
    //    _offscreenCommandBuffers[i]->resetCommandBuffer();
    //}

    //vkResetCommandBuffer(_shadowCommandBuffers[_currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

    //recordShadowCommandBuffer(_shadowCommandBuffers[_currentFrame], imageIndex);
    recordOffscreenCommandBuffer(_offscreenCommandBuffers[_currentFrame]->getVkCommandBuffer(), imageIndex);
    recordCommandBuffer(_commandBuffers[_currentFrame]->getVkCommandBuffer(), imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::array<VkCommandBuffer, 2> submitCommands = { _commandBuffers[_currentFrame]->getVkCommandBuffer(), _offscreenCommandBuffers[_currentFrame]->getVkCommandBuffer()};
    submitInfo.commandBufferCount = static_cast<uint32_t>(submitCommands.size());
    submitInfo.pCommandBuffers = submitCommands.data();

    VkSemaphore signalSemaphores[] = { _renderFinishedSemaphores[_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(_logicalDevice->getGraphicsQueue(), 1, &submitInfo, _inFlightFences[_currentFrame]) != VK_SUCCESS) {
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
        return _physicalDevice->getPropertyManager().checkTextureFormatSupport(format, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
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

void OffscreenRendering::createCommandBuffers() {
    _commandPool = std::make_unique<CommandPool>(*_logicalDevice);
    _commandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    _offscreenCommandBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _commandBuffers.emplace_back(_commandPool->createPrimaryCommandBuffer());
        _offscreenCommandBuffers.emplace_back(_commandPool->createPrimaryCommandBuffer());
    }
}

void OffscreenRendering::updateUniformBuffer(uint32_t currentFrame) {
    const auto& swapchainExtent = _swapchain->getExtent();

    _ubCamera.view = _camera->getViewMatrix();
    _ubCamera.proj = _camera->getProjectionMatrix();
    _ubCamera.pos = _camera->getPosition();

    _dynamicUniformBuffersCamera->updateUniformBuffer(&_ubCamera, currentFrame);

    // std::cout << _ubCamera.pos.x << " " << _ubCamera.pos.y << " " << _ubCamera.pos.z << std::endl;
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
    renderPassInfo.framebuffer = _lowResFramebuffers[imageIndex]->getVkFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    VkExtent2D extent = _swapchain->getExtent();
    extent = { extent.width / 4, extent.height / 4 };
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

    // TBN OBJECT
    vkCmdBindPipeline(commandBuffer, _lowResGraphicsPipeline->getVkPipelineBindPoint(), _lowResGraphicsPipeline->getVkPipeline());

    for (const auto& object : _objects) {
        VkBuffer vertexBuffers[] = { object.vertexBufferPTNTB->getVkBuffer() };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer->getVkBuffer(), 0, object.indexBuffer->getIndexType());

        object._descriptorSet->bindDescriptorSet(commandBuffer, *_lowResGraphicsPipeline, { _currentFrame, object.dynamicUniformIndex });

        vkCmdDrawIndexed(commandBuffer, object.indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }

    // SKYBOX
    vkCmdBindPipeline(commandBuffer, _lowResGraphicsPipelineSkybox->getVkPipelineBindPoint(), _lowResGraphicsPipelineSkybox->getVkPipeline());

    VkBuffer vertexBuffersCube[] = { _vertexBufferCube->getVkBuffer() };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersCube, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBufferCube->getVkBuffer(), 0, _indexBufferCube->getIndexType());

    _descriptorSetSkybox->bindDescriptorSet(commandBuffer, *_lowResGraphicsPipelineSkybox, { _currentFrame });

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
    renderPassInfo.framebuffer = _framebuffers[imageIndex]->getVkFramebuffer();
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

    // TBN OBJECT
    vkCmdBindPipeline(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(), _graphicsPipeline->getVkPipeline());
    for (const auto& object : _objects) {

        VkBuffer vertexBuffers[] = { object.vertexBufferPTNTB->getVkBuffer() };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer->getVkBuffer(), 0, object.indexBuffer->getIndexType());

        object._descriptorSet->bindDescriptorSet(commandBuffer, *_graphicsPipeline, { _currentFrame, object.dynamicUniformIndex });

        vkCmdDrawIndexed(commandBuffer, object.indexBuffer->getIndexCount(), 1, 0, 0, 0);

    }

    // SKYBOX
    vkCmdBindPipeline(commandBuffer, _graphicsPipelineSkybox->getVkPipelineBindPoint(), _graphicsPipelineSkybox->getVkPipeline());

    VkBuffer vertexBuffersCube[] = { _vertexBufferCube->getVkBuffer() };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffersCube, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBufferCube->getVkBuffer(), 0, _indexBufferCube->getIndexType());

    _descriptorSetSkybox->bindDescriptorSet(commandBuffer, *_graphicsPipelineSkybox, { _currentFrame });

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

    VkExtent2D extent = { _shadowMap->getImage().width, _shadowMap->getImage().height };

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _shadowRenderPass->getVkRenderPass();
    renderPassInfo.framebuffer = _shadowFramebuffer->getVkFramebuffer();
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
    // OBJECT TBN
    vkCmdBindPipeline(commandBuffer, _shadowPipeline->getVkPipelineBindPoint(), _shadowPipeline->getVkPipeline());
    for (const auto& object : _objects) {
        VkBuffer vertexBuffers[] = { object.vertexBufferP->getVkBuffer() };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, object.indexBuffer->getVkBuffer(), 0, object.indexBuffer->getIndexType());
        _descriptorSetShadow->bindDescriptorSet(commandBuffer, *_shadowPipeline, { object.dynamicUniformIndex });
        vkCmdDrawIndexed(commandBuffer, object.indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
    //if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to record command buffer!");
    //}
}

void OffscreenRendering::recreateSwapChain() {
    VkExtent2D extent{};
    while (extent.width == 0 || extent.height == 0) {
        extent = _window->getFramebufferSize();
        //glfwWaitEvents();
    }

    _camera->setAspectRatio(static_cast<float>(extent.width) / extent.height);

    vkDeviceWaitIdle(_logicalDevice->getVkDevice());

    _swapchain->recrete();

    _framebuffers.clear();
    _framebufferTextures = std::move(createTexturesFromRenderpass(*_renderPass, extent));
    size_t swapchainImagesCount = _swapchain->getImagesCount();
    for (size_t i = 0; i < swapchainImagesCount; i++) {
        std::vector<VkImageView> imageViews;
        std::transform(_framebufferTextures.cbegin(), _framebufferTextures.cend(), std::back_inserter(imageViews), [this, i](const std::unique_ptr<Texture>& texture) { return texture ? texture->getImage().view : _swapchain->getImages()[i].getImage().view; });
        _framebuffers.emplace_back(std::make_unique<Framebuffer>(*_renderPass, extent, imageViews));
    }
}