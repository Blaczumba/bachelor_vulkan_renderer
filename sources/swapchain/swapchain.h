#pragma once

#include "memory_objects/texture/texture.h"

#include <vulkan/vulkan.h>

#include <vector>

class Window;
class LogicalDevice;
class PhysicalDevice;
class Texture;

namespace {
	struct SwapchainImages {
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
	};
}

class Swapchain {
	VkSwapchainKHR _swapchain;
	const LogicalDevice& _logicalDevice;

	VkSurfaceFormatKHR _surfaceFormat;
	VkExtent2D _extent;
	SwapchainImages _swapchainContainer;

public:
	Swapchain(const LogicalDevice& logicalDevice);
	~Swapchain();

	const VkSwapchainKHR getVkSwapchain() const;
	const VkFormat getVkFormat() const;
	VkExtent2D getExtent() const;
	uint32_t getImagesCount() const;
	const std::vector<VkImage>& getVkImages() const;
	const std::vector<VkImageView>& getVkImageViews() const;

	void cleanup();
	void create();
	void recrete();

	uint32_t imageIndex; // TODO

	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
	VkResult present(uint32_t imageIndex, VkSemaphore waitSemaphore) const;
};