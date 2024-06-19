#include "framebuffer.h"
#include "memory_objects/image.h"

#include <stdexcept>

Framebuffer::Framebuffer(std::shared_ptr<LogicalDevice> logicaldevice, std::shared_ptr<Swapchain> swapchain, std::shared_ptr<Renderpass> renderpass)
    : _logicalDevice(logicaldevice), _swapchain(swapchain), _renderPass(renderpass) {

    const auto& swapchainImageViews = _swapchain->getImageViews();
    const auto& swapchainExtent = _swapchain->getExtent();
    const auto& layout = _renderPass->getLayout();

    for (size_t i = 0; i < swapchainImageViews.size(); i++) {
        std::vector<VkImageView> attachments;
        for (const auto& description : layout) {
            switch (description.finalLayout) {

            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                attachments.push_back(swapchainImageViews[i]);
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                const SwapchainImage imageData = createColorResources(description);
                _images.push_back(imageData);
                attachments.push_back(imageData._resourcesView);
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                const SwapchainImage imageData = createDepthResources(description);
                _images.push_back(imageData);
                attachments.push_back(imageData._resourcesView);
                break;
            }
        }

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass->getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;
        framebufferInfo.layers = 1;

        // resize framebuffers!!!
        if (vkCreateFramebuffer(_logicalDevice->getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }


    //for (size_t i = 0; i < swapchainImageViews.size(); i++) {
    //    std::array<VkImageView, 3> attachments = {
    //        colorImageView,
    //        depthImageView,
    //        swapchainImageViews[i]
    //    };

    //    VkFramebufferCreateInfo framebufferInfo{};
    //    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //    framebufferInfo.renderPass = _renderPass->getVkRenderPass();
    //    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    //    framebufferInfo.pAttachments = attachments.data();
    //    framebufferInfo.width = swapchainExtent.width;
    //    framebufferInfo.height = swapchainExtent.height;
    //    framebufferInfo.layers = 1;

    //    // resize framebuffers!!!
    //    if (vkCreateFramebuffer(_logicalDevice->getVkDevice(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
    //        throw std::runtime_error("failed to create framebuffer!");
    //    }
    //}
}

Framebuffer::~Framebuffer() {
    VkDevice device = _logicalDevice->getVkDevice();

    for (auto& image : _images) {
        vkDestroyImageView(device, image._resourcesView, nullptr);
        vkDestroyImage(device, image._resourcesImage, nullptr);
        vkFreeMemory(device, image._resourcesMemory, nullptr);
    }

    for (auto framebuffer : _framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}

SwapchainImage Framebuffer::createColorResources(const VkAttachmentDescription& description) {
    VkExtent2D swapchainExtent = _swapchain->getExtent();
    VkFormat colorFormat = description.format;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    _logicalDevice->createImage(swapchainExtent.width, swapchainExtent.height, 1, description.samples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
    colorImageView = _logicalDevice->createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    // transitionImageLayout(colorImage, colorFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    return { colorImage, colorImageMemory, colorImageView };
}

SwapchainImage Framebuffer::createDepthResources(const VkAttachmentDescription& description) {
    VkExtent2D swapchainExtent = _swapchain->getExtent();
    VkFormat depthFormat = description.format;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    _logicalDevice->createImage(swapchainExtent.width, swapchainExtent.height, 1, description.samples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = _logicalDevice->createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    if (depthFormat == VK_FORMAT_D32_SFLOAT)
        transitionImageLayout(_logicalDevice.get(), depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    else
        transitionImageLayout(_logicalDevice.get(), depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 1);

    return { depthImage, depthImageMemory, depthImageView };
}
