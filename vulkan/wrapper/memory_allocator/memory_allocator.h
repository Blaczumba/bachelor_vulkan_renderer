#pragma once

#include <set>
#include <unordered_map>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/memory_objects/image.h"

class VmaWrapper {
public:
  VmaWrapper(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance);

  VmaWrapper(VmaWrapper&& allocator) noexcept;

  VmaWrapper& operator=(VmaWrapper&& allocator) noexcept;

  ~VmaWrapper();

  struct Buffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    void* mappedData;
  };

  struct Image {
    VkImage image;
    VmaAllocation allocation;
  };

  Buffer createVkBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
                        VmaAllocationCreateFlags flags = 0U);

  void destroyVkBuffer(VkBuffer buffer, const VmaAllocation allocation);

  void sendDataToBufferMemory(
      VkBuffer buffer, const VmaAllocation allocation, const void* data, size_t size);

  Image createVkImage(const ImageParameters& params, VkImageLayout layout,
                      VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0U);

  void destroyVkImage(VkImage image, const VmaAllocation allocation);

private:
  VmaAllocator _allocator;
};
