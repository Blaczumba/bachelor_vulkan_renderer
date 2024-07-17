#include "double_screenshot_application.h"

#include <algorithm>
#include <array>

DoubleScreenshotApplication::DoubleScreenshotApplication()
	: ApplicationBase() {
    _callbackManager = std::make_unique<FPSCallbackManager>(std::dynamic_pointer_cast<WindowGLFW>(_window));
    _camera = std::make_unique<FPSCamera>(glm::radians(45.0f), 1920.0f / 1080.0f, 0.01f, 10.0f);
    _callbackManager->attach(_camera.get());

    // There must be at least one "Present" attachment (Color or Resolve).
    // There must be the same number of ColorAttachments and ColorResolveAttachments.
    // Every attachment must have the same number of MSAA samples.
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    auto swapchainImageFormat = _swapchain->getVkFormat();
    std::vector<std::unique_ptr<Attachment>> attachments;
    //attachments.emplace_back(std::make_unique<ColorResolvePresentAttachment>(swapchainImageFormat));
    attachments.emplace_back(std::make_unique<ColorPresentAttachment>(swapchainImageFormat));
    attachments.emplace_back(std::make_unique<ColorAttachment>(swapchainImageFormat, msaaSamples));
    attachments.emplace_back(std::make_unique<DepthAttachment>(findDepthFormat(), msaaSamples));

    _renderPass = std::make_shared<Renderpass>(_logicalDevice, std::move(attachments));

    _texture = std::make_shared<Texture2DSampler>(_logicalDevice, TEXTURES_PATH "viking_room.png", _physicalDevice->getMaxSamplerAnisotropy());
    
    _uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
    auto textureUniform = std::make_shared<UniformBufferTexture>(_logicalDevice, _texture, VK_SHADER_STAGE_FRAGMENT_BIT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        std::vector<std::shared_ptr<UniformBufferAbstraction>> ub;
        ub.emplace_back(std::make_shared<UniformBuffer<UniformBufferObject>>(_logicalDevice, VK_SHADER_STAGE_VERTEX_BIT));
        ub.emplace_back(textureUniform);

        _uniformBuffers.emplace_back(std::move(ub));
    }

    _descriptorSets = std::make_unique<DescriptorSets>(_logicalDevice, _uniformBuffers);

    _graphicsPipeline = std::make_unique<GraphicsPipeline>(_logicalDevice, _renderPass, _descriptorSets->getVkDescriptorSetLayout(), msaaSamples, "vert.spv", "frag.spv");
    
    _framebuffer = std::make_unique<Framebuffer>(_logicalDevice, _swapchain, _renderPass);

    _vertexData = _OBJLoader.extract<uint16_t>(MODELS_PATH "viking_room.obj");
    _vertexBuffer = std::make_unique<VertexBuffer<Vertex>>(_logicalDevice, _vertexData.vertices);
    _indexBuffer = std::make_unique<IndexBuffer<uint16_t>>(_logicalDevice, _vertexData.indices);

    _commandBuffers = _logicalDevice->createCommandBuffers(MAX_FRAMES_IN_FLIGHT);

    _screenshot = std::make_unique<Screenshot>(_logicalDevice);

    createSyncObjects();
}

DoubleScreenshotApplication::~DoubleScreenshotApplication() {
    VkDevice device = _logicalDevice->getVkDevice();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, _renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, _imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, _inFlightFences[i], nullptr);
    }
}

DoubleScreenshotApplication& DoubleScreenshotApplication::getInstance() {
    static DoubleScreenshotApplication application;
    return application;
}

void DoubleScreenshotApplication::run() {
    while (_window->open()) {
        _callbackManager->pollEvents();
        draw();
    }
    vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

void DoubleScreenshotApplication::draw() {
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

    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { _imageAvailableSemaphores[_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

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

    //if (glfwGetKey(_window->getGlfwWindow(), GLFW_KEY_P) == GLFW_PRESS) {
    //    // std::async(std::launch::async, &Screenshot::saveScreenshot, _screenshot.get(), "screenshot.ppm", imageIndex);
    //    // _screenshot->saveImage("screenshot.ppm", _swapchain->getImages()[imageIndex]);
    //    const auto& texture = _framebuffer->getColorTextures()[0];
    //    _screenshot->saveImage("screenshot.ppm", texture.getImage());
    //    //std::async(std::launch::async, &Screenshot::saveImage, _screenshot.get(), "screenshot.ppm", texture.getImage());
    //}

    if (++_currentFrame == MAX_FRAMES_IN_FLIGHT)
        _currentFrame = 0;

}

VkFormat DoubleScreenshotApplication::findDepthFormat() const {
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

void DoubleScreenshotApplication::createSyncObjects() {
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

void DoubleScreenshotApplication::updateUniformBuffer(uint32_t currentImage) {
    UniformBufferObject ubo;
    const auto& swapchainExtent = _swapchain->getExtent();

    ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, -1.0f));
    ubo.view = _camera->getViewMatrix();
    ubo.proj = _camera->getProjectionMatrix();

    _uniformBuffers[currentImage][0]->updateUniformBuffer(&ubo);
}

void DoubleScreenshotApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline->getVkPipeline());

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

    VkBuffer vertexBuffers[] = { _vertexBuffer->getVkBuffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, _indexBuffer->getVkBuffer(), 0, _indexBuffer->getIndexType());

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline->getVkPipelineLayout(), 0, 1, &_descriptorSets->getVkDescriptorSet(_currentFrame), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, _indexBuffer->getIndexCount(), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void DoubleScreenshotApplication::recreateSwapChain() {
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