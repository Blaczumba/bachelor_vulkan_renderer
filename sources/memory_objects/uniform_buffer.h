#pragma once

#include <logical_device/logical_device.h>

#include <vulkan/vulkan.h>

#include <memory>
#include <cstring>

template<typename UniformBufferObject>
class UniformBuffer {
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	UniformBuffer(std::shared_ptr<LogicalDevice> logicalDevice);
	~UniformBuffer();

	void updateUniformBuffer(const UniformBufferObject& uniformBufferObject);

	VkBuffer getVkBuffer() const;
};

template<typename UniformBufferObject>
UniformBuffer<UniformBufferObject>::UniformBuffer(std::shared_ptr<LogicalDevice> logicalDevice)
	: _logicalDevice(logicalDevice) {

	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	_logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice->getVkDevice(), _uniformBufferMemory, 0, bufferSize, 0, &_uniformBufferMapped);
}

template<typename UniformBufferObject>
UniformBuffer<UniformBufferObject>::~UniformBuffer() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkDestroyBuffer(device, _uniformBuffer, nullptr);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
}

template<typename UniformBufferObject>
void UniformBuffer<UniformBufferObject>::updateUniformBuffer(const UniformBufferObject& uniformBufferObject) {
	std::memcpy(_uniformBufferMapped, &uniformBufferObject, sizeof(UniformBufferObject));
}

template<typename UniformBufferObject>
VkBuffer UniformBuffer<UniformBufferObject>::getVkBuffer() const {
	return _uniformBuffer;
}