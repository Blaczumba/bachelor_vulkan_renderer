#pragma once

#include "physical_device/device/physical_device.h"
#include "logical_device/logical_device.h"
#include "surface/surface.h"
#include "memory_objects/image.h"

#include <vulkan/vulkan.h>

class Swapchain {
	std::shared_ptr<Surface> _surface;
	std::shared_ptr<Window> _window;
	std::shared_ptr<LogicalDevice> _logicalDevice;
	std::shared_ptr<PhysicalDevice> _physicalDevice;

	std::vector<Image> _images;

	VkSwapchainKHR _swapchain;

public:
	Swapchain(std::shared_ptr<Surface> surface, std::shared_ptr<Window> window, std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<PhysicalDevice> physicalDevice);
	~Swapchain();

	const VkSwapchainKHR getVkSwapchain() const;
	const VkFormat getVkFormat() const;
	VkExtent2D getExtent() const;

	const std::vector<Image>& getImages() const;

	void cleanup();
	void create();
	void recrete();

	uint32_t imageIndex; // TODO

	uint32_t acquireNextImage() const;
	VkResult present(VkSemaphore waitSemaphore) const;
private:
};