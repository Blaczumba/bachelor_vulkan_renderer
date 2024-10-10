#include "logical_device.h"

#include "config/config.h"

#include <set>
#include <stdexcept>

LogicalDevice::LogicalDevice(const PhysicalDevice& physicalDevice)
    : _physicalDevice(physicalDevice) {
    const QueueFamilyIndices& indices = physicalDevice.getPropertyManager().getQueueFamilyIndices();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { 
        indices.graphicsFamily.value(), 
        indices.presentFamily.value(), 
        indices.computeFamily.value(), 
        indices.transferFamily.value() 
    };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {
        .geometryShader = VK_TRUE,
        .tessellationShader = VK_TRUE,
        .sampleRateShading = VK_TRUE,
        .depthClamp = VK_TRUE,
        .samplerAnisotropy = VK_TRUE
    };

    VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
    #ifdef VALIDATION_LAYERS_ENABLED
        .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
        .ppEnabledLayerNames = validationLayers.data(),
    #else
        .enabledLayerCount = 0,
    #endif  // VALIDATION_LAYERS_ENABLED
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    if (vkCreateDevice(_physicalDevice.getVkPhysicalDevice(), &createInfo, nullptr, &_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
    vkGetDeviceQueue(_device, indices.computeFamily.value(), 0, &_computeQueue);
    vkGetDeviceQueue(_device, indices.transferFamily.value(), 0, &_transferQueue);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = indices.graphicsFamily.value()
    };

    if (vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void LogicalDevice::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    const auto& propertyManager = _physicalDevice.getPropertyManager();

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = propertyManager.findMemoryType(memRequirements.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

void LogicalDevice::createImage(Image* image) const {
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = image->format,
        .extent = { 
            .width = image->width,
            .height = image->height,
            .depth = 1
        },
        .mipLevels = image->mipLevels,
        .arrayLayers = image->layerCount,
        .samples = image->numSamples,
        .tiling = image->tiling,
        .usage = image->usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = image->layout
    };

    if (image->layerCount == 6)
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (vkCreateImage(_device, &imageInfo, nullptr, &image->image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    const auto& propertyManager = _physicalDevice.getPropertyManager();

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image->image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = propertyManager.findMemoryType(memRequirements.memoryTypeBits, image->properties)
    };

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &image->memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_device, image->image, image->memory, 0);
}

void LogicalDevice::createImageView(Image* image) const {
    VkImageSubresourceRange range = {
        .aspectMask = image->aspect,
        .baseMipLevel = 0,
        .levelCount = image->mipLevels,
        .baseArrayLayer = 0,
        .layerCount = image->layerCount
    };

    VkImageViewType viewType = (image->layerCount == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

    VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image->image,
        .viewType = viewType,
        .format = image->format,
        .subresourceRange = range
    };

    if (vkCreateImageView(_device, &viewInfo, nullptr, &image->view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }
}

void LogicalDevice::createSampler(Sampler* sampler) const {
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = sampler->magFilter,
        .minFilter = sampler->minFilter,
        .mipmapMode = sampler->mipmapMode,
        .addressModeU = sampler->addressModeU,
        .addressModeV = sampler->addressModeV,
        .addressModeW = sampler->addressModeW,
        .mipLodBias = sampler->mipLodBias,
        .minLod = sampler->minLod,
        .maxLod = sampler->maxLod,
        .borderColor = sampler->borderColor,
        .unnormalizedCoordinates = sampler->unnormalizedCoordinates,
    };

    if (sampler->maxAnisotropy.has_value()) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = sampler->maxAnisotropy.value();
    }

    if (sampler->compareOp.has_value()) {
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = sampler->compareOp.value();
    }
    else {
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    }

    if (vkCreateSampler(_device, &samplerInfo, nullptr, &sampler->sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

VkCommandBuffer LogicalDevice::createCommandBuffer() const {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    return commandBuffer;
}

LogicalDevice::~LogicalDevice() {
    vkDestroyCommandPool(_device, _commandPool, nullptr);
    vkDestroyDevice(_device, nullptr);
}

const VkDevice LogicalDevice::getVkDevice() const {
    return _device;
}

const PhysicalDevice& LogicalDevice::getPhysicalDevice() const {
    return _physicalDevice;
}

const VkQueue LogicalDevice::getGraphicsQueue() const {
    return _graphicsQueue;
}

const VkQueue LogicalDevice::getPresentQueue() const {
    return _presentQueue;
}

const VkQueue LogicalDevice::getComputeQueue() const {
    return _computeQueue;
}

const VkQueue LogicalDevice::getTransferQueue() const {
    return _transferQueue;
}
