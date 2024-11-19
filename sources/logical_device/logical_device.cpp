#include "logical_device.h"

#include "config/config.h"

#include <set>
#include <stdexcept>

LogicalDevice::LogicalDevice(const PhysicalDevice& physicalDevice)
    : _physicalDevice(physicalDevice) {
    const QueueFamilyIndices& indices = physicalDevice.getPropertyManager().getQueueFamilyIndices();

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const std::set<uint32_t> uniqueQueueFamilies = { 
        indices.graphicsFamily.value(), 
        indices.presentFamily.value(), 
        indices.computeFamily.value(), 
        indices.transferFamily.value() 
    };

    float queuePriority = 1.0f;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        queueCreateInfos.emplace_back(
            VkDeviceQueueCreateInfo {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            }
        );
    }

    const VkPhysicalDeviceFeatures deviceFeatures = {
        .geometryShader = VK_TRUE,
        .tessellationShader = VK_TRUE,
        .sampleRateShading = VK_TRUE,
        .depthClamp = VK_TRUE,
        .samplerAnisotropy = VK_TRUE
    };

    const VkDeviceCreateInfo createInfo = {
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

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = propertyManager.findMemoryType(memRequirements.memoryTypeBits, properties)
    };

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

const VkImage LogicalDevice::createImage(const ImageParameters& params) const {
    VkImageCreateInfo imageInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = params.format,
        .extent = { 
            .width = params.width,
            .height = params.height,
            .depth = 1
        },
        .mipLevels = params.mipLevels,
        .arrayLayers = params.layerCount,
        .samples = params.numSamples,
        .tiling = params.tiling,
        .usage = params.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = params.layout
    };

    if (params.layerCount == 6)
        imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    VkImage image;
    if (vkCreateImage(_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    return image;
}

const VkDeviceMemory LogicalDevice::createImageMemory(const VkImage image, const ImageParameters& params) const {
    const auto& propertyManager = _physicalDevice.getPropertyManager();

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image, &memRequirements);

    const VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = propertyManager.findMemoryType(memRequirements.memoryTypeBits, params.properties)
    };

    VkDeviceMemory memory;
    if (vkAllocateMemory(_device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_device, image, memory, 0);
    return memory;
}

const VkImageView LogicalDevice::createImageView(const VkImage image, const ImageParameters& params) const {
    const VkImageSubresourceRange range = {
        .aspectMask = params.aspect,
        .baseMipLevel = 0,
        .levelCount = params.mipLevels,
        .baseArrayLayer = 0,
        .layerCount = params.layerCount
    };

    const VkImageViewType viewType = (params.layerCount == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;

    const VkImageViewCreateInfo viewInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = viewType,
        .format = params.format,
        .subresourceRange = range
    };

    VkImageView view;
    if (vkCreateImageView(_device, &viewInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return view;
}

const VkSampler LogicalDevice::createSampler(const SamplerParameters& params) const {
    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = params.magFilter,
        .minFilter = params.minFilter,
        .mipmapMode = params.mipmapMode,
        .addressModeU = params.addressModeU,
        .addressModeV = params.addressModeV,
        .addressModeW = params.addressModeW,
        .mipLodBias = params.mipLodBias,
        .minLod = params.minLod,
        .maxLod = params.maxLod,
        .borderColor = params.borderColor,
        .unnormalizedCoordinates = params.unnormalizedCoordinates,
    };

    if (params.maxAnisotropy.has_value()) {
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = params.maxAnisotropy.value();
    }

    if (params.compareOp.has_value()) {
        samplerInfo.compareEnable = VK_TRUE;
        samplerInfo.compareOp = params.compareOp.value();
    }
    else {
        samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    }

    VkSampler sampler;
    if (vkCreateSampler(_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

    return sampler;
}

LogicalDevice::~LogicalDevice() {
    vkDestroyDevice(_device, nullptr);
}

const VkDevice LogicalDevice::getVkDevice() const {
    return _device;
}

const PhysicalDevice& LogicalDevice::getPhysicalDevice() const {
    return _physicalDevice;
}

const VkQueue LogicalDevice::getQueue(QueueType queueType) const {
    switch (queueType) {
    case QueueType::GRAPHICS:
        return _graphicsQueue;
    case QueueType::PRESENT:
        return _presentQueue;
    case QueueType::COMPUTE:
        return _computeQueue;
    case QueueType::TRANSFER:
        return _transferQueue;
    default:
        return VK_NULL_HANDLE;
    }
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
