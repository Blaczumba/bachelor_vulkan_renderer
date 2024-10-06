#pragma once

#include "memory_objects/texture/texture.h"

#include <vulkan/vulkan.h>

#include <vector>

class Surface;
class Window;
class LogicalDevice;
class PhysicalDevice;
class Texture;

class Swapchain {
	const Surface& _surface;
	const Window& _window;
	const LogicalDevice& _logicalDevice;

	std::vector<Texture> _images;

	VkSwapchainKHR _swapchain;

public:
	Swapchain(const Surface& surface, const Window& window, const LogicalDevice& logicalDevice);
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
private:
};