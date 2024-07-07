#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <array>
#include <optional>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <variant>
#include <future>

#include <window/window.h>
#include <instance/instance.h>
#include <debug_messenger/debug_messenger.h>
#include <surface/surface.h>
#include <physical_device/device/physical_device.h>
#include <render_pass/render_pass.h>
#include <swapchain/swapchain.h>
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
#include <screenshot/screenshot.h>

const uint32_t WIDTH = 1920;
const uint32_t HEIGHT = 1080;

const int MAX_FRAMES_IN_FLIGHT = 3;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class BejzakEngine {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    std::shared_ptr<Window> _window;
    std::shared_ptr<Instance> _instance;
    std::shared_ptr<DebugMessenger> _debugMessenger;
    std::shared_ptr<Surface> _surface;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<LogicalDevice> _logicalDevice;
    std::shared_ptr<Renderpass> _renderPass;
    std::shared_ptr<Swapchain> _swapchain;

    std::unique_ptr<Framebuffer> _framebuffer;
    std::unique_ptr<VertexBuffer<Vertex>> _vertexBuffer;
    std::unique_ptr<IndexBuffer<uint16_t>> _indexBuffer;
    VertexData<Vertex, uint16_t> _vertexData;
    std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>> _uniformBuffers;
    std::vector<std::vector<std::shared_ptr<UniformBufferAbstraction>>> _uniformBuffers2;
    std::unique_ptr<Pipeline> _graphicsPipeline;
    std::shared_ptr<Texture2D> _texture;
    std::shared_ptr<Texture> _textureDepthAttachment;
    std::shared_ptr<Texture> _textureColorAttachment;
    std::unique_ptr<DescriptorSets> _descriptorSets;
    std::unique_ptr<DescriptorSets> _descriptorSets2;
    std::unique_ptr<Screenshot> _screenshot;

    TinyOBJLoaderVertex vertexLoader;
    std::unique_ptr<CallbackManager> _callbackManager;
    FPSCamera _camera;

    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkPipeline graphicsPipeline;

    // VkCommandPool commandPool;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    //VkDescriptorPool descriptorPool;
    //std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;

    bool framebufferResized = false;

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        _window = std::make_shared<Window>("Bejzak Engine", 1920, 1080);
        _callbackManager = std::make_unique<FPSCallbackManager>(_window, _camera);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
        createRenderPass();
        createTextureImage();
        createUniformBuffers();
        createDescriptorSets();
        createGraphicsPipeline();
        createFramebuffers();
        loadModel();
        createVertexBuffer();
        createIndexBuffer();
        createCommandBuffers();
        createSyncObjects();
        createScreenshotObject();
    }

    void mainLoop() {
        while (_window->open()) {
            _callbackManager->pollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void cleanup() {

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, inFlightFences[i], nullptr);
        }

        // vkFreeCommandBuffers(device, commandPool, MAX_FRAMES_IN_FLIGHT, commandBuffers.data());
        // vkDestroyCommandPool(device, commandPool, nullptr);

        glfwTerminate();
    }

    void recreateSwapChain() {
        Extent extent{};
        while (extent.width == 0 || extent.height == 0) {
            extent = _window->getFramebufferSize();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device);

        _swapchain->recrete();

        createFramebuffers();
    }

    void createInstance() {
        _instance = std::make_shared<Instance>("Bejzak Engine");
    }

    void setupDebugMessenger() {
#ifdef VALIDATION_LAYERS_ENABLED
        _debugMessenger = std::make_shared<DebugMessenger>(_instance);
#endif // VALIDATION_LAYERS_ENABLED
    }

    void createSurface() {
        _surface = std::make_shared<Surface>(_instance, _window);
    }

    void pickPhysicalDevice() {
        _physicalDevice = std::make_shared<PhysicalDevice>(_instance, _surface);
    }

    void createLogicalDevice() {
        _logicalDevice = std::make_shared<LogicalDevice>(_physicalDevice);
        device = _logicalDevice->getVkDevice();
        graphicsQueue = _logicalDevice->graphicsQueue;
        presentQueue = _logicalDevice->presentQueue;
    }

    void createSwapChain() {
        _swapchain = std::make_shared<Swapchain>(_surface, _window, _logicalDevice, _physicalDevice);
    }

    void createRenderPass() {
        // There must be at least one "Present" attachment (Color or Resolve).
        // There must be the same number of ColorAttachments and ColorResolveAttachments.
        // Every attachment must have the same number of MSAA samples.
        msaaSamples = VK_SAMPLE_COUNT_4_BIT;
        auto swapchainImageFormat = _swapchain->getSwapchainImageFormat();
        std::vector<std::unique_ptr<Attachment>> attachments;
        //attachments.emplace_back(std::make_unique<ColorResolvePresentAttachment>(swapchainImageFormat));
        attachments.emplace_back(std::make_unique<ColorResolvePresentAttachment>(swapchainImageFormat));
        attachments.emplace_back(std::make_unique<ColorAttachment>(swapchainImageFormat, msaaSamples));
        attachments.emplace_back(std::make_unique<DepthAttachment>(findDepthFormat(), msaaSamples));

        _renderPass = std::make_shared<Renderpass>(_logicalDevice, std::move(attachments));
    }

    void createGraphicsPipeline() {
        _graphicsPipeline = std::make_unique<GraphicsPipeline>(_logicalDevice, _renderPass, _descriptorSets->getVkDescriptorSetLayout(), msaaSamples);
    }

    void createFramebuffers() {
        _framebuffer = std::make_unique<Framebuffer>(_logicalDevice, _swapchain, _renderPass);
        //VkExtent2D extent = _swapchain->getExtent();
        //// _textureColorAttachment = std::make_shared<Texture2DColor>(_logicalDevice, _swapchain->getSwapchainImageFormat(), msaaSamples, extent);
        //_textureDepthAttachment = std::make_shared<Texture2DDepth>(_logicalDevice, VK_FORMAT_D32_SFLOAT, msaaSamples, extent);

        //auto swapchainImages = _swapchain->getImageViews();
        //uint32_t imageCount = swapchainImages.size();
        //std::vector<std::vector<VkImageView>> views = {
        //    {swapchainImages[0], _textureDepthAttachment->getVkImageView()},
        //    {swapchainImages[1], _textureDepthAttachment->getVkImageView()},
        //    {swapchainImages[2], _textureDepthAttachment->getVkImageView()},
        //    {swapchainImages[3], _textureDepthAttachment->getVkImageView()},
        //};
        //_framebuffer = std::make_unique<Framebuffer>(_logicalDevice, std::move(views), _renderPass, _swapchain->getExtent(), imageCount);
    }

    void loadModel() {
        _vertexData = vertexLoader.extract<uint16_t>(MODELS_PATH "viking_room.obj");
    }

    void createVertexBuffer() {
        _vertexBuffer = std::make_unique<VertexBuffer<Vertex>>(_logicalDevice, _vertexData.vertices);
    }

    void createIndexBuffer() {
        _indexBuffer = std::make_unique<IndexBuffer<uint16_t>>(_logicalDevice, _vertexData.indices);
    }

    void createUniformBuffers() {
        _uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
        _uniformBuffers2.reserve(MAX_FRAMES_IN_FLIGHT);
        auto textureUniform = std::make_shared<UniformBufferTexture>(_logicalDevice, _texture);

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            std::vector<std::shared_ptr<UniformBufferAbstraction>> ub;
            ub.emplace_back(std::make_shared<UniformBuffer<UniformBufferObject>>(_logicalDevice));
            ub.emplace_back(textureUniform);

            _uniformBuffers.emplace_back(std::move(ub));
        }
    }

    void createDescriptorSets() {
        _descriptorSets = std::make_unique<DescriptorSets>(_logicalDevice, _uniformBuffers);
    }

    VkFormat findDepthFormat() {
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

    void createTextureImage() {
        _texture = std::make_shared<Texture2D>(_logicalDevice, TEXTURES_PATH "viking_room.png", _physicalDevice->getMaxSamplerAnisotropy());
    }

    void createCommandBuffers() {
        commandBuffers = _logicalDevice->createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
    }

    void createScreenshotObject() {
        _screenshot = std::make_unique<Screenshot>(_physicalDevice, _logicalDevice, _swapchain);
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline->getVkPipelineLayout(), 0, 1, &_descriptorSets->getVkDescriptorSet(currentFrame), 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(_vertexData.indices.size()), 1, 0, 0, 0);

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        const auto& swapchainExtent = _swapchain->getExtent();

        UniformBufferObject ubo;
        ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 1.0f, -1.0f));
        //ubo.model = glm::mat4(1.0f);
        ubo.view = _camera.getViewMatrix();

        ubo.proj = glm::perspective(glm::radians(45.0f), swapchainExtent.width / (float)swapchainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] = -ubo.proj[1][1];

        _uniformBuffers[currentImage][0]->updateUniformBuffer(&ubo);
        // _uniformBuffers[currentImage][1]->updateUniformBuffer(&ubo);
        // memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }
    void drawFrame() {

        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;

        VkResult result = vkAcquireNextImageKHR(device, _swapchain->getVkSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }


        updateUniformBuffer(currentFrame);


        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        recordCommandBuffer(commandBuffers[currentFrame], imageIndex); 

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { _swapchain->getVkSwapchain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        if (glfwGetKey(_window->getGlfwWindow(), GLFW_KEY_P) == GLFW_PRESS)
            // std::async(std::launch::async, &Screenshot::saveScreenshot, _screenshot.get(), "screenshot.ppm", imageIndex);
            std::async(std::launch::async, &Screenshot::saveImage, _screenshot.get(), "screenshot.ppm", _texture->getVkImage(), _texture->getVkExtent());

        if (++currentFrame == MAX_FRAMES_IN_FLIGHT)
            currentFrame = 0;

    }
};

int main() {
    BejzakEngine app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}