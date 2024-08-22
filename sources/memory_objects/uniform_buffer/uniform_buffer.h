#pragma once

#include <logical_device/logical_device.h>
#include <memory_objects/texture/texture_2D_sampler.h>

#include <vulkan/vulkan.h>

#include <memory>
#include <cstring>

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
	VkDescriptorImageInfo _imageInfo{};

protected:
	const Texture2DSampler& _texture;

public:
	UniformBufferTexture(const Texture2DSampler& texture) 
		: UniformBuffer(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER), _texture(texture) {
		const Image& image = _texture.getImage();

		_imageInfo.sampler = _texture.getVkSampler();
		_imageInfo.imageView = image.view;
		_imageInfo.imageLayout = image.layout;

		_size = 0; // TODO set exact size
	}

	virtual ~UniformBufferTexture() = default;
	const Texture2DSampler* getTexturePtr() const { return &_texture; }

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override {
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = _type;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &_imageInfo;
		
		return descriptorWrite;
	}
};

template<typename UniformBufferType>
class UniformBufferStruct : public UniformBuffer {
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	VkDescriptorBufferInfo _bufferInfo;

	const LogicalDevice& _logicalDevice;

public:
	UniformBufferStruct(const LogicalDevice& logicalDevice);
	~UniformBufferStruct();

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
	void updateUniformBuffer(UniformBufferType* object);
};

template<typename UniformBufferType>
UniformBufferStruct<UniformBufferType>::UniformBufferStruct(const LogicalDevice& logicalDevice)
	: UniformBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER), _logicalDevice(logicalDevice) {

	const PhysicalDevice& physicalDevice = _logicalDevice.getPhysicalDevice();

	const auto& limits = physicalDevice.getPropertyManager().getPhysicalDeviceLimits();
	size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;

	_size = sizeof(UniformBufferType);
	if (minUboAlignment > 0) {
		_size = (_size + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	_logicalDevice.createBuffer(_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice.getVkDevice(), _uniformBufferMemory, 0, _size, 0, &_uniformBufferMapped);

	_bufferInfo.buffer = _uniformBuffer;
	_bufferInfo.offset = 0;
	_bufferInfo.range = sizeof(UniformBufferType);
}

template<typename UniformBufferType>
UniformBufferStruct<UniformBufferType>::~UniformBufferStruct() {
	const VkDevice device = _logicalDevice.getVkDevice();

	vkUnmapMemory(device, _uniformBufferMemory);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, _uniformBuffer, nullptr);
}

template<typename UniformBufferType>
VkWriteDescriptorSet UniformBufferStruct<UniformBufferType>::getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = _type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &_bufferInfo;

	return descriptorWrite;
}

template<typename UniformBufferType>
void UniformBufferStruct<UniformBufferType>::updateUniformBuffer(UniformBufferType* object) {
	std::memcpy(_uniformBufferMapped, object, sizeof(UniformBufferType));
	
	VkMappedMemoryRange memoryRange{};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = _uniformBufferMemory;
	memoryRange.size = _size;
	vkFlushMappedMemoryRanges(_logicalDevice.getVkDevice(), 1, &memoryRange);
}

template<typename UniformBufferType>
class UniformBufferDynamic : public UniformBuffer {
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	VkDescriptorBufferInfo _bufferInfo;

	const LogicalDevice& _logicalDevice;

	uint32_t _count;

public:
	UniformBufferDynamic(const LogicalDevice& logicalDevice, uint32_t count);
	~UniformBufferDynamic();

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
	void updateUniformBuffer(UniformBufferType* object, uint32_t index);
	void makeUpdatesVisible();
	void makeUpdatesVisible(uint32_t objectIndex);

};

template<typename UniformBufferType>
UniformBufferDynamic<UniformBufferType>::UniformBufferDynamic(const LogicalDevice& logicalDevice, uint32_t count)
	: UniformBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC), _logicalDevice(logicalDevice), _count(count) {

	const PhysicalDevice& physicalDevice = _logicalDevice.getPhysicalDevice();

	const auto& limits = physicalDevice.getPropertyManager().getPhysicalDeviceLimits();
	size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;

	_size = sizeof(UniformBufferType);
	if (minUboAlignment > 0) {
		_size = (_size + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	const VkDeviceSize bufferSize = _count * _size;

	_logicalDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice.getVkDevice(), _uniformBufferMemory, 0, bufferSize, 0, &_uniformBufferMapped);

	_bufferInfo.buffer = _uniformBuffer;
	_bufferInfo.offset = 0;
	_bufferInfo.range = sizeof(UniformBufferType);
}

template<typename UniformBufferType>
UniformBufferDynamic<UniformBufferType>::~UniformBufferDynamic() {
	const VkDevice device = _logicalDevice.getVkDevice();

	vkUnmapMemory(device, _uniformBufferMemory);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, _uniformBuffer, nullptr);
}

template<typename UniformBufferType>
VkWriteDescriptorSet UniformBufferDynamic<UniformBufferType>::getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = _type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &_bufferInfo;

	return descriptorWrite;
}

template<typename UniformBufferType>
void UniformBufferDynamic<UniformBufferType>::updateUniformBuffer(UniformBufferType* object, uint32_t index) {
	std::memcpy(static_cast<uint8_t*>(_uniformBufferMapped) + index * _size, object, sizeof(UniformBufferType));
}

template<typename UniformBufferType>
void UniformBufferDynamic<UniformBufferType>::makeUpdatesVisible() {
	VkMappedMemoryRange memoryRange{};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = _uniformBufferMemory;
	memoryRange.size = _count * _size;
	vkFlushMappedMemoryRanges(_logicalDevice.getVkDevice(), 1, &memoryRange);
}

template<typename UniformBufferType>
void UniformBufferDynamic<UniformBufferType>::makeUpdatesVisible(uint32_t objectIndex) {
	VkMappedMemoryRange memoryRange{};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = _uniformBufferMemory;
	memoryRange.size = _size;
	memoryRange.offset = objectIndex * _size;
	vkFlushMappedMemoryRanges(_logicalDevice.getVkDevice(), 1, &memoryRange);
}

template<typename UniformBufferType>
class UniformBufferInlineBlock : public UniformBuffer {
	UniformBufferType _data;
public:
	UniformBufferInlineBlock(const UniformBufferType& object);
	~UniformBufferInlineBlock() = default;

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override;
	// void updateUniformBuffer(UniformBufferType* object);
};

template<typename UniformBufferType>
UniformBufferInlineBlock<UniformBufferType>::UniformBufferInlineBlock(const UniformBufferType& object)
	: UniformBuffer(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK), _data(object) {
	_size = sizeof(UniformBufferType);
}

template<typename UniformBufferType>
VkWriteDescriptorSet UniformBufferInlineBlock<UniformBufferType>::getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const {
	static VkWriteDescriptorSetInlineUniformBlock inlineUniformBlock = {};
	inlineUniformBlock.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_INLINE_UNIFORM_BLOCK;
	inlineUniformBlock.dataSize = sizeof(UniformBufferType);
	inlineUniformBlock.pData = &_data;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = binding;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = _type;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pNext = &inlineUniformBlock;

	return descriptorWrite;
}

