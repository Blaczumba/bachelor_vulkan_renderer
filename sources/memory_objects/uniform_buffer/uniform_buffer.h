#pragma once

#include <logical_device/logical_device.h>
#include <memory_objects/texture/texture.h>

#include <vulkan/vulkan.h>

#include <memory>

static VkDeviceSize getMemoryAlignment(size_t size, size_t minUboAlignment) {
	return minUboAlignment > 0 ? (size + minUboAlignment - 1) & ~(minUboAlignment - 1) : size;
}

class UniformBuffer {
protected:
	const VkDescriptorType _type;
	uint32_t _size;

public:
	UniformBuffer(VkDescriptorType type) : _type(type) {}
	virtual ~UniformBuffer() = default;

	virtual VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const =0;

	VkDescriptorType getVkDescriptorType() const { return _type; }
	uint32_t getSize() const { return _size; }

};

class UniformBufferTexture : public UniformBuffer {
	const Texture& _texture;
	const VkDescriptorImageInfo _imageInfo;

public:
	UniformBufferTexture(const Texture& texture) 
		: UniformBuffer(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER), _texture(texture), 
		_imageInfo{ texture.getVkSampler(), texture.getVkImageView(), texture.getImageParameters().layout } {

	}

	~UniformBufferTexture() override = default;
	const Texture* getTexturePtr() const { return &_texture; }

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override {
		return VkWriteDescriptorSet {
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = binding,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = _type,
			.pImageInfo = &_imageInfo
		};
	}
};

template<typename UniformBufferType>
class UniformBufferData : public UniformBuffer {
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	VkDescriptorBufferInfo _bufferInfo;
	const uint32_t _count;

	const LogicalDevice& _logicalDevice;

public:
	UniformBufferData(const LogicalDevice& logicalDevice, uint32_t count = 1U);
	~UniformBufferData() override;

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
	void updateUniformBuffer(const UniformBufferType* object, uint32_t index = 0);
};

template<typename UniformBufferType>
UniformBufferData<UniformBufferType>::UniformBufferData(const LogicalDevice& logicalDevice, uint32_t count)
	: UniformBuffer(count > 1 ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER),
	_logicalDevice(logicalDevice), _count(count) {
	const PhysicalDevice& physicalDevice = _logicalDevice.getPhysicalDevice();
	const auto& limits = physicalDevice.getPropertyManager().getPhysicalDeviceLimits();

	_size = getMemoryAlignment(sizeof(UniformBufferType), limits.minUniformBufferOffsetAlignment);
	const VkDeviceSize bufferSize = VkDeviceSize{ _count }*_size;
	_uniformBuffer = _logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
	_uniformBufferMemory = _logicalDevice.createBufferMemory(_uniformBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	
	vkMapMemory(_logicalDevice.getVkDevice(), _uniformBufferMemory, 0, bufferSize, 0, &_uniformBufferMapped);
	
	_bufferInfo = VkDescriptorBufferInfo{
		.buffer = _uniformBuffer,
		.offset = 0,
		.range = _size
	};
}

template<typename UniformBufferType>
UniformBufferData<UniformBufferType>::~UniformBufferData() {
	const VkDevice device = _logicalDevice.getVkDevice();

	vkUnmapMemory(device, _uniformBufferMemory);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, _uniformBuffer, nullptr);
}

template<typename UniformBufferType>
VkWriteDescriptorSet UniformBufferData<UniformBufferType>::getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	return VkWriteDescriptorSet {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = descriptorSet,
		.dstBinding = binding,
		.dstArrayElement = 0,
		.descriptorCount = 1,
		.descriptorType = _type,
		.pBufferInfo = &_bufferInfo
	};
}

template<typename UniformBufferType>
void UniformBufferData<UniformBufferType>::updateUniformBuffer(const UniformBufferType* object, uint32_t index) {
	std::memcpy(static_cast<uint8_t*>(_uniformBufferMapped) + size_t{ index }*_size, object, sizeof(UniformBufferType));
}
