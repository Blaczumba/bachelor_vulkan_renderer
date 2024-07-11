#pragma once

#include <vector>

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    // VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME
};