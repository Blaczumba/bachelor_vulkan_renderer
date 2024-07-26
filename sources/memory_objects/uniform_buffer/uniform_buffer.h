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
	std::shared_ptr<Texture2DSampler> _texture;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	UniformBufferTexture(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Texture2DSampler> texture) 
		: UniformBuffer(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER), _logicalDevice(logicalDevice), _texture(texture) {
		const Image& image = _texture->getImage();

		_imageInfo.sampler = _texture->getVkSampler();
		_imageInfo.imageView = image.view;
		_imageInfo.imageLayout = image.layout;

		_size = 0; // TODO set exact size
	}

	virtual ~UniformBufferTexture() = default;
	const Texture2DSampler* getTexturePtr() const { return _texture.get(); }

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

class UniformBufferData : public UniformBuffer {
protected:
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	VkDescriptorBufferInfo _bufferInfo;

	std::shared_ptr<LogicalDevice> _logicalDevice;

public:
	UniformBufferData(std::shared_ptr<LogicalDevice> logicalDevice, VkDescriptorType type) : UniformBuffer(type), _logicalDevice(logicalDevice) {}
	virtual ~UniformBufferData() = default;

	VkWriteDescriptorSet getVkWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding) const override {
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

	virtual VkBuffer getVkBuffer() const { return _uniformBuffer; }
};

template<typename UniformBufferType>
class UniformBufferStruct : public UniformBufferData {
public:
	UniformBufferStruct(std::shared_ptr<LogicalDevice> logicalDevice);
	~UniformBufferStruct();

	void updateUniformBuffer(UniformBufferType* object);
};

template<typename UniformBufferType>
UniformBufferStruct<UniformBufferType>::UniformBufferStruct(std::shared_ptr<LogicalDevice> logicalDevice)
	: UniformBufferData(logicalDevice, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

	_size = sizeof(UniformBufferType);

	_logicalDevice->createBuffer(_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice->getVkDevice(), _uniformBufferMemory, 0, _size, 0, &_uniformBufferMapped);

	_bufferInfo.buffer	= _uniformBuffer;
	_bufferInfo.range	= _size;
	_bufferInfo.offset	= 0;
}

template<typename UniformBufferType>
UniformBufferStruct<UniformBufferType>::~UniformBufferStruct() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkUnmapMemory(device, _uniformBufferMemory);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, _uniformBuffer, nullptr);
}

template<typename UniformBufferType>
void UniformBufferStruct<UniformBufferType>::updateUniformBuffer(UniformBufferType* object) {
	std::memcpy(_uniformBufferMapped, object, sizeof(UniformBufferType));
	
	//VkMappedMemoryRange memoryRange{};
	//memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	//memoryRange.memory = _uniformBufferMemory;
	//memoryRange.size = sizeof(UniformBufferType);
	//vkFlushMappedMemoryRanges(_logicalDevice->getVkDevice(), 1, &memoryRange);
}

template<typename UniformBufferType>
class UniformBufferDynamic : public UniformBufferData {
	uint32_t _count;

public:
	UniformBufferDynamic(std::shared_ptr<LogicalDevice> logicalDevice, uint32_t count);
	~UniformBufferDynamic();

	void updateUniformBuffer(UniformBufferType* object, uint32_t index);
	void makeUpdatesVisible();

};

template<typename UniformBufferType>
UniformBufferDynamic<UniformBufferType>::UniformBufferDynamic(std::shared_ptr<LogicalDevice> logicalDevice, uint32_t count)
	: UniformBufferData(logicalDevice, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC), _count(count) {

	const PhysicalDevice& physicalDevice = _logicalDevice->getPhysicalDevice();

	const auto limits = physicalDevice.getPhysicalDeviceLimits();
	size_t minUboAlignment = limits.minUniformBufferOffsetAlignment;

	_size = sizeof(UniformBufferType);
	if (minUboAlignment > 0) {
		_size = (_size + minUboAlignment - 1) & ~(minUboAlignment - 1);
	}

	const VkDeviceSize bufferSize = _count * _size;

	_logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice->getVkDevice(), _uniformBufferMemory, 0, bufferSize, 0, &_uniformBufferMapped);

	_bufferInfo.buffer = _uniformBuffer;
	_bufferInfo.range = static_cast<uint32_t>(sizeof(UniformBufferType));
	_bufferInfo.offset = 0;
}

template<typename UniformBufferType>
UniformBufferDynamic<UniformBufferType>::~UniformBufferDynamic() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkUnmapMemory(device, _uniformBufferMemory);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
	vkDestroyBuffer(device, _uniformBuffer, nullptr);
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
	vkFlushMappedMemoryRanges(_logicalDevice->getVkDevice(), 1, &memoryRange);
}