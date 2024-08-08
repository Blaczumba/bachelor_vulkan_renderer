#pragma once

#include <vulkan/vulkan.h>

#include <vector>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, VkFormat preferredFormat = VK_FORMAT_B8G8R8A8_SRGB);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferredMode = VK_PRESENT_MODE_FIFO_KHR);
VkExtent2D chooseSwapExtent(VkExtent2D actualWindowExtent, const VkSurfaceCapabilitiesKHR& capabilities);