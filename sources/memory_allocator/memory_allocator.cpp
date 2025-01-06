#include "memory_objects/texture/texture.h"
#include "memory_allocator.h"

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#include <variant>

VmaWrapper::VmaWrapper(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkInstance instance) {
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.flags = // VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT |
		VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT |
		VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
		VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;

	vmaCreateAllocator(&allocatorCreateInfo, &_allocator);
}

VmaWrapper::VmaWrapper(VmaWrapper&& allocator) {
	_allocator = std::exchange(allocator._allocator, nullptr);
}

VmaWrapper::~VmaWrapper() {
	if (_allocator) {
		vmaDestroyAllocator(_allocator);
	}
}

std::tuple<VkBuffer, VmaAllocation, void*> VmaWrapper::createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) {
	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	const VmaAllocationCreateInfo vmaallocInfo = {
		.flags = flags,
		.usage = memoryUsage
	};

	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
	if (vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, &allocationInfo) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	};
	return { buffer, allocation, allocationInfo.pMappedData };
}

void VmaWrapper::destroyVkBuffer(const VkBuffer buffer, const VmaAllocation allocation) {
	vmaDestroyBuffer(_allocator, buffer, allocation);
}

void VmaWrapper::sendDataToBufferMemory(const VkBuffer buffer, const VmaAllocation allocation, const void* data, size_t size) {
	vmaCopyMemoryToAllocation(_allocator, data, allocation, 0, size);
}

std::pair<VkImage, VmaAllocation> VmaWrapper::createVkImage(const ImageParameters& params, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags) {
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

	const VmaAllocationCreateInfo vmaAllocInfo = {
		.flags = flags,
		.usage = memoryUsage
	};

	VmaAllocation allocation;
	VkImage image;
	vmaCreateImage(_allocator, &imageInfo, &vmaAllocInfo, &image, &allocation, nullptr);
	return { image, allocation };
}

void VmaWrapper::destroyVkImage(const VkImage image, const VmaAllocation allocation) {
	vmaDestroyImage(_allocator, image, allocation);
}
