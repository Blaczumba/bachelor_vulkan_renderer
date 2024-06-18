#include <vulkan/vulkan.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferredMode = VK_PRESENT_MODE_FIFO_KHR);
VkExtent2D chooseSwapExtent(VkExtent2D actualWindowExtent, const VkSurfaceCapabilitiesKHR& capabilities);