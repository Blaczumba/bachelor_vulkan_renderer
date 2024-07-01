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
// #include <image/image.h>

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
    std::vector<std::vector<std::unique_ptr<UniformBufferAbstraction>>> _uniformBuffers;
    std::unique_ptr<Pipeline> _graphicsPipeline;
    std::shared_ptr<Texture> _texture;
    std::unique_ptr<DescriptorSets> _descriptorSets;

    TinyOBJLoaderVertex<uint16_t> vertexLoader;

    VertexData<Vertex, uint16_t> _vertexData;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

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
    }

    void mainLoop() {
        while (_window->open()) {
            glfwPollEvents();
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
        swapChain = _swapchain->_swapchain;
        swapChainExtent = _swapchain->_extent;
        swapChainImageFormat = _swapchain->_imageFormat;

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
        msaaSamples = _physicalDevice->getMaxMsaaSampleCount();
        msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        physicalDevice = _physicalDevice->getVkPhysicalDevice();
    }

    void createLogicalDevice() {
        _logicalDevice = std::make_shared<LogicalDevice>(_physicalDevice);
        device = _logicalDevice->getVkDevice();
        graphicsQueue = _logicalDevice->graphicsQueue;
        presentQueue = _logicalDevice->presentQueue;
    }

    void createSwapChain() {
        _swapchain = std::make_shared<Swapchain>(_surface, _window, _logicalDevice, _physicalDevice);
        swapChain = _swapchain->_swapchain;
        swapChainExtent = _swapchain->_extent;
        swapChainImageFormat = _swapchain->_imageFormat;
    }

    void createRenderPass() {
        // There must be at least one "Present" attachment (Color or Resolve).
        // There must be the same number of ColorAttachments and ColorResolveAttachments.
        // Every attachment must have the same number of MSAA samples.
        std::vector<std::unique_ptr<Attachment>> attachments;
        attachments.emplace_back(std::make_unique<ColorPresentAttachment>(swapChainImageFormat));
        attachments.emplace_back(std::make_unique<ColorAttachment>(swapChainImageFormat, msaaSamples));
        attachments.emplace_back(std::make_unique<ColorAttachment>(swapChainImageFormat, msaaSamples));
        attachments.emplace_back(std::make_unique<ColorAttachment>(swapChainImageFormat, msaaSamples));
        attachments.emplace_back(std::make_unique<DepthAttachment>(findDepthFormat(), msaaSamples));

        _renderPass = std::make_shared<Renderpass>(_logicalDevice, std::move(attachments));
    }

    void createGraphicsPipeline() {
        _graphicsPipeline = std::make_unique<GraphicsPipeline>(_logicalDevice, _renderPass, _descriptorSets->getVkDescriptorSetLayout(), msaaSamples);
    }

    void createFramebuffers() {
        _framebuffer = std::make_unique<Framebuffer>(_logicalDevice, _swapchain, _renderPass);
    }


    void loadModel() {
        _vertexData = vertexLoader.extract(MODELS_PATH "viking_room.obj");
    }

    void createVertexBuffer() {
        _vertexBuffer = std::make_unique<VertexBuffer<Vertex>>(_logicalDevice, _vertexData.vertices);
    }

    void createIndexBuffer() {
        _indexBuffer = std::make_unique<IndexBuffer<uint16_t>>(_logicalDevice, _vertexData.indices);
    }

    void createUniformBuffers() {
        _uniformBuffers.reserve(MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            std::vector<std::unique_ptr<UniformBufferAbstraction>> ub;
            ub.emplace_back(std::make_unique<UniformBuffer<UniformBufferObject>>(_logicalDevice));
            ub.emplace_back(std::make_unique<UniformBufferTexture>(_logicalDevice, _texture));
            
            _uniformBuffers.emplace_back(std::move(ub));
        }
    }

    void createDescriptorSets() {
        _descriptorSets = std::make_unique<DescriptorSets>(_logicalDevice, _uniformBuffers);
    }

    VkFormat findDepthFormat() {
        std::array<VkFormat, 3> depthFormats = {
            VK_FORMAT_D24_UNORM_S8_UINT,
            VK_FORMAT_D32_SFLOAT,
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
        renderPassInfo.renderArea.extent = swapChainExtent;

        const auto& clearValues = _renderPass->getClearValues();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline->getVkPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)swapChainExtent.width;
        viewport.height = (float)swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = swapChainExtent;
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

        UniformBufferObject ubo;
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.3f, 1.3, 1.3));
        //ubo.model = glm::mat4(1.0f);
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] = -ubo.proj[1][1];

        _uniformBuffers[currentImage][0]->updateUniformBuffer(&ubo);
        // _uniformBuffers[currentImage][1]->updateUniformBuffer(&ubo);
        // memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void drawFrame() {
        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

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

        VkSwapchainKHR swapChains[] = { swapChain };
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

        if (++currentFrame == MAX_FRAMES_IN_FLIGHT)
            currentFrame -= MAX_FRAMES_IN_FLIGHT;
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