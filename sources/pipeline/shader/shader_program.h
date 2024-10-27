#pragma once

#include "descriptor_set/descriptor_set_layout.h"
#include "memory_objects/uniform_buffer/push_constants.h"
#include "primitives/primitives.h"
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

template<typename VertexType>
class GraphicsShaderProgram : public ShaderProgram {
protected:
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;

public:
	GraphicsShaderProgram(const LogicalDevice& logicalDevice) : ShaderProgram(logicalDevice) {}
	static VkPipelineVertexInputStateCreateInfo getVkPipelineVertexInputStateCreateInfo() {
		static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexType>();
		static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexType>();

		return VkPipelineVertexInputStateCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};
	}
};

class PBRShaderProgram : public GraphicsShaderProgram<VertexPTNT> {
public:
	PBRShaderProgram(const LogicalDevice& logicalDevice);
};

class PBRTesselationShaderProgram : public GraphicsShaderProgram<VertexPTNT> {
public:
	PBRTesselationShaderProgram(const LogicalDevice& logicalDevice);
};

class SkyboxShaderProgram : public GraphicsShaderProgram<VertexP> {
public:
	SkyboxShaderProgram(const LogicalDevice& logicalDevice);
};

class ShadowShaderProgram : public GraphicsShaderProgram<VertexP> {
public:
	ShadowShaderProgram(const LogicalDevice& logicalDevice);
};

class SkyboxOffscreenShaderProgram : public GraphicsShaderProgram<VertexP> {
public:
	SkyboxOffscreenShaderProgram(const LogicalDevice& logicalDevice);
};

class PBRShaderOffscreenProgram : public GraphicsShaderProgram<VertexPTNT> {
public:
	PBRShaderOffscreenProgram(const LogicalDevice& logicalDevice);
};