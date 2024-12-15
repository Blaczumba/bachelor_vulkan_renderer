#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

#include <unordered_map>
#include <set>

struct ImageParameters; // TODO Why I need to declare it if it is included

class VmaWrapper {
public:
	VmaWrapper(const VkDevice device, const VkPhysicalDevice physicalDevice, const VkInstance instance);
	VmaWrapper(const VmaWrapper& allocator) = delete;
	VmaWrapper(VmaWrapper&& allocator);
	~VmaWrapper();

	void destroy() {
		vmaDestroyAllocator(_allocator);
		_allocator = nullptr;
	}

	const VkBuffer createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
	void destroyVkBuffer(const VkBuffer buffer);
	void sendDataToBufferMemory(const VkBuffer buffer, const void* data, size_t size);
	const VkImage createVkImage(const ImageParameters& params, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0U);
	void destroyVkImage(const VkImage image);

private:
	VmaAllocator _allocator;
	std::unordered_map<VkBuffer, VmaAllocation> _bufferAllocations;
	std::unordered_map<VkImage, VmaAllocation> _imageAllocations;
};
