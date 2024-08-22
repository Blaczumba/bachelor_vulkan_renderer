#pragma once

#include <vulkan/vulkan.h>

#include <string>

class LogicalDevice;

class Shader {
	VkShaderModule _shaderModule;
	const VkShaderStageFlagBits _shaderStage;
	const std::string_view _name;

	const LogicalDevice& _logicalDevice;

public:
	Shader(const LogicalDevice& logicalDevice, const std::string& shaderPath, const VkShaderStageFlagBits shaderStage);
	~Shader();

	VkPipelineShaderStageCreateInfo getVkPipelineStageCreateInfo() const;

	const VkShaderModule getVkShaderModule() const;
	VkShaderStageFlagBits getVkShaderStageFlagBits() const;
	const std::string_view getName() const;
};
