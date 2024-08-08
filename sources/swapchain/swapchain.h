#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Surface;
class Window;
class LogicalDevice;
class PhysicalDevice;
struct Image;

class Swapchain {
	const Surface& _surface;
	const Window& _window;
	const LogicalDevice& _logicalDevice;
	const PhysicalDevice& _physicalDevice;

	std::vector<Image> _images;

	VkSwapchainKHR _swapchain;

public:
	Swapchain(const Surface& surface, const Window& window, const LogicalDevice& logicalDevice, const PhysicalDevice& physicalDevice);
	~Swapchain();

	const VkSwapchainKHR getVkSwapchain() const;
	const VkFormat getVkFormat() const;
	VkExtent2D getExtent() const;

	const std::vector<Image>& getImages() const;

	void cleanup();
	void create();
	void recrete();

	uint32_t imageIndex; // TODO

	VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex) const;
	VkResult present(uint32_t imageIndex, VkSemaphore waitSemaphore) const;
private:
};