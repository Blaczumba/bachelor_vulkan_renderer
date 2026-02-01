#include "memory_allocator.h"

#include "vulkan/wrapper/memory_objects/image.h"
#include "vulkan/wrapper/util/check.h"

#define VMA_IMPLEMENTATION
#include <variant>
#include <vma/vk_mem_alloc.h>

VmaWrapper::VmaWrapper(VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance) {
  const VmaAllocatorCreateInfo allocatorCreateInfo = {
    // .flags = // VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT |
    // VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT |
    // VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
    // VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    .physicalDevice = physicalDevice,
    .device = device,
    .instance = instance};
  vmaCreateAllocator(&allocatorCreateInfo, &_allocator);
}

VmaWrapper::VmaWrapper(VmaWrapper&& allocator) noexcept
  : _allocator(std::exchange(allocator._allocator, nullptr)) {}

VmaWrapper& VmaWrapper::operator=(VmaWrapper&& allocator) noexcept {
  if (this == &allocator) {
    return *this;
  }
  // TODO what if _allocator != nullptr
  _allocator = std::exchange(allocator._allocator, nullptr);
  return *this;
}

VmaWrapper::~VmaWrapper() {
  if (_allocator) {
    vmaDestroyAllocator(_allocator);
  }
}

VmaWrapper::Buffer VmaWrapper::createVkBuffer(
    VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags) {
  const VkBufferCreateInfo bufferInfo = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

  const VmaAllocationCreateInfo vmaallocInfo = {.flags = flags, .usage = memoryUsage};

  VkBuffer buffer;
  VmaAllocation allocation;
  VmaAllocationInfo allocationInfo;
  CHECK_VKCMD(vmaCreateBuffer(
                  _allocator, &bufferInfo, &vmaallocInfo, &buffer, &allocation, &allocationInfo),
              "Failed to create VkBuffer by VMA.");
  return VmaWrapper::Buffer{buffer, allocation, allocationInfo.pMappedData};
}

void VmaWrapper::destroyVkBuffer(VkBuffer buffer, const VmaAllocation allocation) {
  vmaDestroyBuffer(_allocator, buffer, allocation);
}

void VmaWrapper::sendDataToBufferMemory(
    VkBuffer buffer, const VmaAllocation allocation, const void* data, size_t size) {
  vmaCopyMemoryToAllocation(_allocator, data, allocation, 0, size);
}

VmaWrapper::Image VmaWrapper::createVkImage(
    const VkImageCreateInfo& imageCreateInfo, VmaMemoryUsage memoryUsage,
    VmaAllocationCreateFlags flags) {
  const VmaAllocationCreateInfo vmaAllocInfo = {.flags = flags, .usage = memoryUsage};
  VmaAllocation allocation;
  VkImage image;
  CHECK_VKCMD(
      vmaCreateImage(_allocator, &imageCreateInfo, &vmaAllocInfo, &image, &allocation, nullptr),
              "Failed to create VkImage by VMA.");
  return VmaWrapper::Image{image, allocation};
}

void VmaWrapper::destroyVkImage(const VkImage image, const VmaAllocation allocation) {
  vmaDestroyImage(_allocator, image, allocation);
}
