#pragma once

#include "shader.h"
#include "descriptor_set/descriptor_set_layout.h"
#include "memory_objects/uniform_buffer/push_constants.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class LogicalDevice;

class ShaderProgram {
protected:
	std::unique_ptr<DescriptorSetLayout> _descriptorSetLayout;
	std::vector<Shader> _shaders;
	
	const LogicalDevice& _logicalDevice;
	PushConstants _pushConstants;

public:
	ShaderProgram(const LogicalDevice& logicalDevice);
	std::vector<VkPipelineShaderStageCreateInfo> getVkPipelineShaderStageCreateInfos() const;

	const DescriptorSetLayout& getDescriptorSetLayout() const;
	const PushConstants& getPushConstants() const;
};

class GraphicsShaderProgram : public ShaderProgram {
protected:
	VkVertexInputBindingDescription _bindingDescription;
	std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;

public:
	GraphicsShaderProgram(const LogicalDevice& logicalDevice);

	const VkVertexInputBindingDescription& getVkVertexInputBindingDescription() const;
	const std::vector<VkVertexInputAttributeDescription>& getVkVertexInputAttributeDescriptions() const;
};

class PBRShaderProgram : public GraphicsShaderProgram {
public:
	PBRShaderProgram(const LogicalDevice& logicalDevice);
};

class SkyboxShaderProgram : public GraphicsShaderProgram {
public:
	SkyboxShaderProgram(const LogicalDevice& logicalDevice);
};

class ShadowShaderProgram : public GraphicsShaderProgram {
public:
	ShadowShaderProgram(const LogicalDevice& logicalDevice);
};

class SkyboxOffscreenShaderProgram : public GraphicsShaderProgram {
public:
	SkyboxOffscreenShaderProgram(const LogicalDevice& logicalDevice);
};

class PBRShaderOffscreenProgram : public GraphicsShaderProgram {
public:
	PBRShaderOffscreenProgram(const LogicalDevice& logicalDevice);
};