#include "swapchain_utils.h"

#include <algorithm>

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    auto availableFormat = std::find_if(availableFormats.cbegin(), availableFormats.cend(), [](const auto& format) {
        return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        });

    return (availableFormat != availableFormats.cend()) ? *availableFormat : availableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, VkPresentModeKHR preferredMode) {
    auto availablePresentMode = std::find_if(availablePresentModes.cbegin(), availablePresentModes.cend(), [=](const auto& mode) {
        return mode == preferredMode;
        });

    return (availablePresentMode != availablePresentModes.cend()) ? *availablePresentMode : VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D chooseSwapExtent(VkExtent2D actualWindowExtent, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        actualWindowExtent.width = std::clamp(actualWindowExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualWindowExtent.height = std::clamp(actualWindowExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualWindowExtent;
    }
}
