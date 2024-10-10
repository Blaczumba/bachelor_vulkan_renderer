#pragma once

#include "descriptor_set/descriptor_set_layout.h"
#include "memory_objects/uniform_buffer/push_constants.h"
#include "shader.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

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
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;

public:
	GraphicsShaderProgram(const LogicalDevice& logicalDevice);
	const VkPipelineVertexInputStateCreateInfo& getVkPipelineVertexInputStateCreateInfo() const;
};

class PBRShaderProgram : public GraphicsShaderProgram {
public:
	PBRShaderProgram(const LogicalDevice& logicalDevice);
};

class PBRTesselationShaderProgram : public GraphicsShaderProgram {
public:
	PBRTesselationShaderProgram(const LogicalDevice& logicalDevice);
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