#pragma once

#include "memory_objects/texture/texture.h"

#include <vulkan/vulkan.h>

#include <vector>

class Window;
class LogicalDevice;
class PhysicalDevice;
class Texture;

class Swapchain {
	VkSwapchainKHR _swapchain;
	const LogicalDevice& _logicalDevice;

	std::vector<Texture> _images;

public:
	Swapchain(const LogicalDevice& logicalDevice);
	~Swapchain();

	const VkSwapchainKHR getVkSwapchain() const;
	const VkFormat getVkFormat() const;
	VkExtent2D getExtent() const;

	const std::vector<Texture>& getImages() const;

	void cleanup();
	void create();
	void recrete();

	uint32_t imageIndex; // TODO

	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
	VkResult present(uint32_t imageIndex, VkSemaphore waitSemaphore) const;
};