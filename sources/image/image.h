#pragma once

#include <vulkan/vulkan.h>

void transitionImageLayout(LogicalDevice* logicalDevice, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectFlags, uint32_t mipLevels);