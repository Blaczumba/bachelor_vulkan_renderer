#include "double_screenshot_application.h"

#include "memory_objects/texture/texture_factory.h"
#include "model_loader/tiny_gltf_loader/tiny_gltf_loader.h"
#include "entity_component_system/system/movement_system.h"
#include "entity_component_system/component/material.h"
#include "entity_component_system/component/mesh.h"
#include "entity_component_system/component/position.h"
#include "entity_component_system/component/transform.h"
#include "entity_component_system/component/velocity.h"
#include "pipeline/shader/shader_program.h"
#include "render_pass/attachment/attachment_layout.h"
#include "thread_pool/thread_pool.h"

#include <algorithm>
#include <array>
#include <chrono>

SingleApp::SingleApp()
    : ApplicationBase() {
    _newVertexDataTBN = LoadGLTF<VertexPTNT, uint16_t>(MODELS_PATH "sponza/scene.gltf");

    createDescriptorSets();
    loadObjects();
    createPresentResources();
    createShadowResources();

    VertexData<VertexP, uint16_t> vertexDataCube = TinyOBJLoaderVertex::extract<VertexP, uint16_t>(MODELS_PATH "cube.obj");
    _vertexBufferCube = std::make_unique<VertexBuffer>(*_singleTimeCommandPool, vertexDataCube.vertices);
    _indexBufferCube = std::make_unique<IndexBuffer>(*_singleTimeCommandPool, vertexDataCube.indices);

    createCommandBuffers();

    //_screenshot = std::make_unique<Screenshot>(*_logicalDevice);
    //_screenshot->addImageToObserved(_framebufferTextures[1]->getImage(), "hig_res_screenshot.ppm");
    _camera = std::make_unique<FPSCamera>(glm::radians(45.0f), 1920.0f / 1080.0f, 0.01f, 100.0f);

    _callbackManager = std::make_unique<FPSCallbackManager>(_window.get());
    _callbackManager->attach(_camera.get());
    //_callbackManager->attach(_screenshot.get());

    createSyncObjects();
}

void SingleApp::loadObjects() {
    const auto& propertyManager = _physicalDevice->getPropertyManager();
    float maxSamplerAnisotropy = propertyManager.getMaxSamplerAnisotropy();
    uint32_t index = 0;

    _objects.reserve(_newVertexDataTBN.size());
    for (uint32_t i = 0; i < _newVertexDataTBN.size(); i++) {
        Entity e = _registry.createEntity();
        if (_newVertexDataTBN[i].normalTextures.empty() || _newVertexDataTBN[i].metallicRoughnessTextures.empty())
            continue;
        const std::string diffusePath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].diffuseTextures[0];
        const std::string metallicRoughnessPath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].metallicRoughnessTextures[0];
        const std::string normalPath = std::string(MODELS_PATH) + "sponza/" + _newVertexDataTBN[i].normalTextures[0];

        if (!_uniformMap.contains(diffusePath)) {
            _textures.emplace_back(TextureFactory::create2DTextureImage(*_singleTimeCommandPool, diffusePath, VK_FORMAT_R8G8B8A8_SRGB, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(diffusePath, std::make_shared<UniformBufferTexture>(*_textures.back())));
        }
        if (!_uniformMap.contains(normalPath)) {
            _textures.emplace_back(TextureFactory::create2DTextureImage(*_singleTimeCommandPool, normalPath, VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(normalPath, std::make_shared<UniformBufferTexture>(*_textures.back())));
        }
        if (!_uniformMap.contains(metallicRoughnessPath)) {
            _textures.emplace_back(TextureFactory::create2DTextureImage(*_singleTimeCommandPool, metallicRoughnessPath, VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy));
            _uniformMap.emplace(std::make_pair(metallicRoughnessPath, std::make_shared<UniformBufferTexture>(*_textures.back())));
        }

        auto descriptorSet = _descriptorPool->createDesriptorSet();
        descriptorSet->updateDescriptorSet({ _dynamicUniformBuffersCamera.get(), _uniformMap[diffusePath].get(), _uniformBuffersLight.get(), _uniformBuffersObjects.get(), _shadowTextureUniform.get(), _uniformMap[normalPath].get(), _uniformMap[metallicRoughnessPath].get() });

        std::vector<glm::vec3> pVertexData;
        std::transform(_newVertexDataTBN[i].vertices.cbegin(), _newVertexDataTBN[i].vertices.cend(), std::back_inserter(pVertexData), [](const VertexPTNT& vertex) { return vertex.pos; });

        _objects.emplace_back("Object", e);

        MeshComponent msh;
        msh.vertexBuffer = std::make_shared<VertexBuffer>(*_singleTimeCommandPool, _newVertexDataTBN[i].vertices);
        msh.indexBuffer = std::make_shared<IndexBuffer>(*_singleTimeCommandPool, _newVertexDataTBN[i].indices);
        msh.vertexBufferPrimitive = std::make_shared<VertexBuffer>(*_singleTimeCommandPool, pVertexData);
        msh.aabb = createAABBfromVertices(pVertexData, _newVertexDataTBN[i].model);
        _registry.addComponent<MeshComponent>(e, std::move(msh));

        TransformComponent trsf;
        trsf.model = _newVertexDataTBN[i].model;
        _registry.addComponent<TransformComponent>(e, std::move(trsf));

        _entityToIndex.emplace(e, index);
        _entitytoDescriptorSet.emplace(e, std::move(descriptorSet));
        
        _ubObject.model = _newVertexDataTBN[i].model;
        _uniformBuffersObjects->updateUniformBuffer(&_ubObject, index++);
    }

    AABB sceneAABB = _registry.getComponent<MeshComponent>(_objects[0].getEntity()).aabb;
    for (int i = 1; i < _objects.size(); i++) {
        sceneAABB.extend(_registry.getComponent<MeshComponent>(_objects[i].getEntity()).aabb);
    }
    _octree = std::make_unique<Octree>(sceneAABB);

    for (const auto& object : _objects)
        _octree->addObject(&object, _registry.getComponent<MeshComponent>(object.getEntity()).aabb);
}

void SingleApp::createDescriptorSets() {
    const auto& propertyManager = _physicalDevice->getPropertyManager();
    float maxSamplerAnisotropy = propertyManager.getMaxSamplerAnisotropy();
    _textureCubemap = TextureFactory::createCubemap(*_singleTimeCommandPool, TEXTURES_PATH "cubemap_yokohama_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, maxSamplerAnisotropy);
    _shadowMap = TextureFactory::create2DShadowmap(*_singleTimeCommandPool, 1024 * 2, 1024 * 2, VK_FORMAT_D32_SFLOAT);

    _uniformBuffersObjects = std::make_unique<UniformBufferData<UniformBufferObject>>(*_logicalDevice, _newVertexDataTBN.size());
    _uniformBuffersLight = std::make_unique<UniformBufferData<UniformBufferLight>>(*_logicalDevice);
    _dynamicUniformBuffersCamera = std::make_unique<UniformBufferData<UniformBufferCamera>>(*_logicalDevice, MAX_FRAMES_IN_FLIGHT);

    _skyboxTextureUniform = std::make_unique<UniformBufferTexture>(*_textureCubemap);
    _shadowTextureUniform = std::make_unique<UniformBufferTexture>(*_shadowMap);

    _pbrShaderProgram = ShaderProgramFactory::createShaderProgram(ShaderProgramType::PBR, *_logicalDevice);
    _skyboxShaderProgram = ShaderProgramFactory::createShaderProgram(ShaderProgramType::SKYBOX, *_logicalDevice);
    _shadowShaderProgram = ShaderProgramFactory::createShaderProgram(ShaderProgramType::SHADOW, *_logicalDevice);

    _descriptorPool = std::make_shared<DescriptorPool>(*_logicalDevice, _pbrShaderProgram->getDescriptorSetLayout(), 150);
    _descriptorPoolSkybox = std::make_shared<DescriptorPool>(*_logicalDevice, _skyboxShaderProgram->getDescriptorSetLayout(), 1);
    _descriptorPoolShadow = std::make_shared<DescriptorPool>(*_logicalDevice, _shadowShaderProgram->getDescriptorSetLayout(), 1);

    _descriptorSetSkybox = _descriptorPoolSkybox->createDesriptorSet();
    _descriptorSetShadow = _descriptorPoolShadow->createDesriptorSet();

    _descriptorSetSkybox->updateDescriptorSet({ _dynamicUniformBuffersCamera.get(), _skyboxTextureUniform.get() });
    _descriptorSetShadow->updateDescriptorSet({ _uniformBuffersLight.get(),  _uniformBuffersObjects.get() });

    _ubLight.pos = glm::vec3(15.1891f, 2.66408f, -0.841221f);
    _ubLight.projView = glm::perspective(glm::radians(120.0f), 1.0f, 0.01f, 40.0f);
    _ubLight.projView[1][1] = -_ubLight.projView[1][1];
    _ubLight.projView = _ubLight.projView * glm::lookAt(_ubLight.pos, glm::vec3(-3.82383f, 3.66503f, 1.30751f), glm::vec3(0.0f, 1.0f, 0.0f));
    _uniformBuffersLight->updateUniformBuffer(&_ubLight);
}

void SingleApp::createPresentResources() {
    const VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_4_BIT;
    const VkFormat swapchainImageFormat = _swapchain->getVkFormat();
    const VkExtent2D extent = _swapchain->getExtent();

    AttachmentLayout attachmentsLayout;
    attachmentsLayout.addColorResolvePresentAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE);
    attachmentsLayout.addColorResolveAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE);
    attachmentsLayout.addColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples);
    attachmentsLayout.addColorAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples);
    attachmentsLayout.addDepthAttachment(findDepthFormat(), VK_ATTACHMENT_STORE_OP_DONT_CARE, msaaSamples);

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

    for (uint8_t i = 0; i < _swapchain->getImagesCount(); ++i) {
        _framebuffers.emplace_back(std::make_unique<Framebuffer>(*_renderPass, *_swapchain, i, *_singleTimeCommandPool));
    }

    const GraphicsPipelineParameters pbrParameters = {
        .msaaSamples = msaaSamples,
        // .patchControlPoints = 3,
    };
    _graphicsPipeline = std::make_unique<GraphicsPipeline>(*_renderPass, *_pbrShaderProgram, pbrParameters);

    const GraphicsPipelineParameters skyboxParameters = {
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .msaaSamples = msaaSamples
    };
    _graphicsPipelineSkybox = std::make_unique<GraphicsPipeline>(*_renderPass, *_skyboxShaderProgram, skyboxParameters);
}

void SingleApp::createShadowResources() {
    const VkSampleCountFlagBits shadowMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    const VkFormat imageFormat = VK_FORMAT_D32_SFLOAT;
    const VkExtent2D extent = { 1024 * 2, 1024 * 2 };
    AttachmentLayout attachmentLayout;
    attachmentLayout.addShadowAttachment(imageFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    Subpass subpass(attachmentLayout);
    subpass.addSubpassOutputAttachment(0);

    _shadowRenderPass = std::make_shared<Renderpass>(*_logicalDevice, attachmentLayout);
    _shadowRenderPass->addSubpass(subpass);
    _shadowRenderPass->create();

    _shadowFramebuffer = std::make_unique<Framebuffer>(*_shadowRenderPass, std::vector<std::shared_ptr<Texture>>{ _shadowMap });

    const GraphicsPipelineParameters parameters = {
        .depthBiasConstantFactor = 0.7f,
        .depthBiasSlopeFactor = 2.0f
    };
    _shadowPipeline = std::make_unique<GraphicsPipeline>(*_shadowRenderPass, *_shadowShaderProgram, parameters);
}

SingleApp::~SingleApp() {
    const VkDevice device = _logicalDevice->getVkDevice();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, _inFlightFences[i], nullptr);
    }
}

SingleApp& SingleApp::getInstance() {
    static SingleApp application;
    return application;
}

void SingleApp::run() {
    updateUniformBuffer(_currentFrame);
    {
        SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
        VkCommandBuffer commandBuffer = handle.getCommandBuffer();
        recordShadowCommandBuffer(commandBuffer, 0);
    }

    for (auto& object : _objects) {
        _registry.getComponent<MeshComponent>(object.getEntity()).vertexBufferPrimitive.reset();
    }

    _threadPool = std::make_unique<ThreadPool>(MAX_THREADS_IN_POOL);

    while (_window->open()) {
        _callbackManager->pollEvents();
        draw();
    }
    vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

void SingleApp::draw() {
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

    _primaryCommandBuffer[_currentFrame]->resetCommandBuffer();
    for(int i = 0; i < MAX_THREADS_IN_POOL; i++)
        _commandBuffers[_currentFrame][i]->resetCommandBuffer();

    //recordShadowCommandBuffer(_shadowCommandBuffers[_currentFrame], imageIndex);
    recordCommandBuffer(imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    std::array<VkCommandBuffer, 1> submitCommands = { _primaryCommandBuffer[_currentFrame]->getVkCommandBuffer() };
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

VkFormat SingleApp::findDepthFormat() const {
    const std::array<VkFormat, 3> depthFormats = {
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

void SingleApp::createSyncObjects() {
    _imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    _inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    const VkDevice device = _logicalDevice->getVkDevice();

    const VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    const VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}
int first = 0;
void SingleApp::updateUniformBuffer(uint32_t currentFrame) {
    _ubCamera.view = _camera->getViewMatrix();
    _ubCamera.proj = _camera->getProjectionMatrix();
    _ubCamera.pos = _camera->getPosition();
    _dynamicUniformBuffersCamera->updateUniformBuffer(&_ubCamera, currentFrame);
}

void SingleApp::createCommandBuffers() {
    _commandPool.reserve(MAX_THREADS_IN_POOL + 1);
    for (int i = 0; i < MAX_THREADS_IN_POOL + 1; i++) {
        _commandPool.emplace_back(std::make_unique<CommandPool>(*_logicalDevice));
    }

    _primaryCommandBuffer.reserve(MAX_FRAMES_IN_FLIGHT);
    _commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    _shadowCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        _primaryCommandBuffer.emplace_back(_commandPool[MAX_THREADS_IN_POOL]->createPrimaryCommandBuffer());
        _commandBuffers[i].reserve(MAX_THREADS_IN_POOL);
        _shadowCommandBuffers[i].reserve(MAX_THREADS_IN_POOL);
        for (int j = 0; j < MAX_THREADS_IN_POOL; j++) {
            _commandBuffers[i].emplace_back(_commandPool[j]->createSecondaryCommandBuffer());
            _shadowCommandBuffers[i].emplace_back(_commandPool[j]->createSecondaryCommandBuffer());
        }
    }
}

void SingleApp::recordOctreeSecondaryCommandBuffer(const VkCommandBuffer commandBuffer, const OctreeNode* rootNode, const std::array<glm::vec4, NUM_CUBE_FACES>& planes) {
    if (!rootNode || !rootNode->getVolume().intersectsFrustum(planes)) return;

    static std::queue<const OctreeNode*> nodeQueue;
    nodeQueue.push(rootNode);

    static constexpr OctreeNode::Subvolume options[] = {
        OctreeNode::Subvolume::LOWER_LEFT_BACK, OctreeNode::Subvolume::LOWER_LEFT_FRONT,
        OctreeNode::Subvolume::LOWER_RIGHT_BACK, OctreeNode::Subvolume::LOWER_RIGHT_FRONT,
        OctreeNode::Subvolume::UPPER_LEFT_BACK, OctreeNode::Subvolume::UPPER_LEFT_FRONT,
        OctreeNode::Subvolume::UPPER_RIGHT_BACK, OctreeNode::Subvolume::UPPER_RIGHT_FRONT
    };

    while (!nodeQueue.empty()) {
        const OctreeNode* node = nodeQueue.front();
        nodeQueue.pop();

        for (const Object* object : node->getObjects()) {
            const auto& meshComponent = _registry.getComponent<MeshComponent>(object->getEntity());
            const IndexBuffer& indexBuffer = *meshComponent.indexBuffer;
            const VertexBuffer& vertexBuffer = *meshComponent.vertexBuffer;
            vertexBuffer.bind(commandBuffer);
            indexBuffer.bind(commandBuffer);
            _entitytoDescriptorSet[object->getEntity()]->bind(commandBuffer, *_graphicsPipeline, { _currentFrame, _entityToIndex[object->getEntity()] });
            vkCmdDrawIndexed(commandBuffer, indexBuffer.getIndexCount(), 1, 0, 0, 0);
        }

        for (auto option : options) {
            const OctreeNode* childNode = node->getChild(option);
            if (childNode && childNode->getVolume().intersectsFrustum(planes)) {
                nodeQueue.push(childNode);
            }
        }
    }
}

void SingleApp::recordCommandBuffer(uint32_t imageIndex) {
    const Framebuffer& framebuffer = *_framebuffers[imageIndex];
    const PrimaryCommandBuffer& primaryCommandBuffer = *_primaryCommandBuffer[_currentFrame];
    primaryCommandBuffer.begin();
    primaryCommandBuffer.beginRenderPass(framebuffer);

    const VkExtent2D framebufferExtent = framebuffer.getVkExtent();
    const VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)framebufferExtent.width,
        .height = (float)framebufferExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    const VkRect2D scissor = {
        .offset = { 0, 0 },
        .extent = framebufferExtent
    };

    _threadPool->getThread(0)->addJob([&]() {
        _commandBuffers[_currentFrame][0]->begin(framebuffer);
        VkCommandBuffer commandBuffer = _commandBuffers[_currentFrame][0]->getVkCommandBuffer();
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        const OctreeNode* root = _octree->getRoot();
        const auto& planes = extractFrustumPlanes(_camera->getProjectionMatrix() * _camera->getViewMatrix());

        vkCmdBindPipeline(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(), _graphicsPipeline->getVkPipeline());
        recordOctreeSecondaryCommandBuffer(commandBuffer, root, planes);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    });

    _threadPool->getThread(1)->addJob([&]() {
        // Skybox
        _commandBuffers[_currentFrame][1]->begin(framebuffer);
        VkCommandBuffer commandBuffer = _commandBuffers[_currentFrame][1]->getVkCommandBuffer();
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        vkCmdBindPipeline(commandBuffer, _graphicsPipelineSkybox->getVkPipelineBindPoint(), _graphicsPipelineSkybox->getVkPipeline());

        _vertexBufferCube->bind(commandBuffer);
        _indexBufferCube->bind(commandBuffer);
        _descriptorSetSkybox->bind(commandBuffer, *_graphicsPipelineSkybox, { _currentFrame });
        vkCmdDrawIndexed(commandBuffer, _indexBufferCube->getIndexCount(), 1, 0, 0, 0);
        vkEndCommandBuffer(commandBuffer);
    });

    _threadPool->wait();

    primaryCommandBuffer.executeSecondaryCommandBuffers({ _commandBuffers[_currentFrame][0]->getVkCommandBuffer(), _commandBuffers[_currentFrame][1]->getVkCommandBuffer() });
    primaryCommandBuffer.endRenderPass();

    if (primaryCommandBuffer.end() != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void SingleApp::recordShadowCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    //if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to begin recording command buffer!");
    //}

    VkExtent2D extent = _shadowMap->getVkExtent2D();
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _shadowRenderPass->getVkRenderPass();
    renderPassInfo.framebuffer = _shadowFramebuffer->getVkFramebuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = extent;

    const auto& clearValues = _shadowRenderPass->getAttachmentsLayout().getVkClearValues();
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
    vkCmdBindPipeline(commandBuffer, _shadowPipeline->getVkPipelineBindPoint(), _shadowPipeline->getVkPipeline());

    for (const auto& object : _objects) {
        const auto& meshComponent = _registry.getComponent<MeshComponent>(object.getEntity());
        VkBuffer vertexBuffers[] = { meshComponent.vertexBufferPrimitive->getVkBuffer() };
        const IndexBuffer* indexBuffer = meshComponent.indexBuffer.get();
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getVkBuffer(), 0, indexBuffer->getIndexType());
        _descriptorSetShadow->bind(commandBuffer, *_shadowPipeline, { _entityToIndex[object.getEntity()] });
        vkCmdDrawIndexed(commandBuffer, indexBuffer->getIndexCount(), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
    //if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to record command buffer!");
    //}
}

void SingleApp::recreateSwapChain() {
    VkExtent2D extent{};
    while (extent.width == 0 || extent.height == 0) {
        extent = _window->getFramebufferSize();
        //glfwWaitEvents();
    }

    _camera->setAspectRatio(static_cast<float>(extent.width) / extent.height);
    vkDeviceWaitIdle(_logicalDevice->getVkDevice());

    _swapchain->recrete();
    for (uint8_t i = 0; i < _swapchain->getImagesCount(); ++i) {
        _framebuffers[i] = std::make_unique<Framebuffer>(*_renderPass, *_swapchain, i, *_singleTimeCommandPool);
    }
}