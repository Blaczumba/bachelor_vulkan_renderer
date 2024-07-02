#pragma once

#include "physical_device/device/physical_device.h"
#include "logical_device/logical_device.h"
#include "surface/surface.h"

#include <vulkan/vulkan.h>

class Swapchain {
	std::shared_ptr<Surface> _surface;
	std::shared_ptr<Window> _window;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<PhysicalDevice> _physicalDevice;

	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _images;
	std::vector<VkImageView> _imageViews;
	VkFormat _imageFormat;
	VkExtent2D _extent;
public:
	Swapchain(std::shared_ptr<Surface> surface, std::shared_ptr<Window> window, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<PhysicalDevice> physicalDevice);
	~Swapchain();

	const VkSwapchainKHR getVkSwapchain() const;
	VkFormat getSwapchainImageFormat() const;
	const std::vector<VkImageView>& getImageViews() const;
	const VkExtent2D& getExtent() const;

	void cleanup();
	void create();
	void recrete();
private:
};