#pragma once

#include "physical_device/physical_device.h"
#include "logical_device/logical_device.h"
#include "surface/surface.h"

class Swapchain {
	std::shared_ptr<Surface> _surface;
	std::shared_ptr<Window> _window;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<PhysicalDevice> _physicalDevice;

	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _images;
	VkFormat _imageFormat;
	VkExtent2D _extent;
	std::vector<VkImageView> _imageViews;
	std::vector<VkFramebuffer> _framebuffers;
public:
	Swapchain(std::shared_ptr<Surface> surface, std::shared_ptr<Window> window, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<PhysicalDevice> physicalDevice);
private:
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferredMode = VK_PRESENT_MODE_FIFO_KHR);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
};