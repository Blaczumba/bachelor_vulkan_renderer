#pragma once

#include <logical_device/logical_device.h>
#include <memory_objects/texture/texture_2D_sampler.h>

#include <vulkan/vulkan.h>

#include <memory>
#include <cstring>

class UniformBufferAbstraction {
public:
	virtual ~UniformBufferAbstraction() = default;

	virtual void updateUniformBuffer(void* object) =0;

	virtual VkDescriptorType getVkDescriptorType() const =0;
};

class UniformBufferTexture : public UniformBufferAbstraction {
protected:
	std::shared_ptr<Texture2DSampler> _texture;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	UniformBufferTexture(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Texture2DSampler> texture) 
		: _logicalDevice(logicalDevice), _texture(texture) {}
	virtual ~UniformBufferTexture() = default;
	const Texture2DSampler* getTexturePtr() const { return _texture.get(); }

	void updateUniformBuffer(void* object) override {};

	VkDescriptorType getVkDescriptorType() const override { 
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}
};

class UniformBufferStruct : public UniformBufferAbstraction {
protected:
	VkBuffer _uniformBuffer;
	VkDeviceMemory _uniformBufferMemory;
	void* _uniformBufferMapped;

	std::shared_ptr<LogicalDevice> _logicalDevice;
public:
	UniformBufferStruct(std::shared_ptr<LogicalDevice> logicalDevice) : _logicalDevice(logicalDevice) {}
	virtual ~UniformBufferStruct() = default;
	virtual VkBuffer getVkBuffer() const { return _uniformBuffer; }

	void updateUniformBuffer(void* object) =0;

	virtual VkDescriptorType getVkDescriptorType() const =0;
	virtual size_t getBufferSize() const =0;
};

template<typename UniformBufferType>
class UniformBuffer : public UniformBufferStruct {
public:
	UniformBuffer(std::shared_ptr<LogicalDevice> logicalDevice);
	~UniformBuffer();

	void updateUniformBuffer(void* object) override;

	VkDescriptorType getVkDescriptorType() const override {
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}

	size_t getBufferSize() const override {
		return sizeof(UniformBufferType);
	}
};

template<typename UniformBufferType>
UniformBuffer<UniformBufferType>::UniformBuffer(std::shared_ptr<LogicalDevice> logicalDevice)
	: UniformBufferStruct(logicalDevice) {

	VkDeviceSize bufferSize = sizeof(UniformBufferType);

	_logicalDevice->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniformBuffer, _uniformBufferMemory);

	vkMapMemory(_logicalDevice->getVkDevice(), _uniformBufferMemory, 0, bufferSize, 0, &_uniformBufferMapped);
}

template<typename UniformBufferType>
UniformBuffer<UniformBufferType>::~UniformBuffer() {
	VkDevice device = _logicalDevice->getVkDevice();

	vkDestroyBuffer(device, _uniformBuffer, nullptr);
	vkFreeMemory(device, _uniformBufferMemory, nullptr);
}

template<typename UniformBufferType>
void UniformBuffer<UniformBufferType>::updateUniformBuffer(void* object) {
	std::memcpy(_uniformBufferMapped, object, sizeof(UniformBufferType));
}
