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
	ShaderProgram(const LogicalDevice& logicalDevice, std::vector<Shader>&& shaders, std::unique_ptr<DescriptorSetLayout>&& descriptorSetLayout);
	std::vector<VkPipelineShaderStageCreateInfo> getVkPipelineShaderStageCreateInfos() const;

	const DescriptorSetLayout& getDescriptorSetLayout() const;
	const PushConstants& getPushConstants() const;
};

class GraphicsShaderProgram : public ShaderProgram {
protected:
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;

public:
    template<typename VertexType>
    GraphicsShaderProgram(const LogicalDevice& logicalDevice, std::vector<Shader>&& shaders, std::unique_ptr<DescriptorSetLayout>&& descriptorSetLayout, VertexType)
        : ShaderProgram(logicalDevice, std::move(shaders), std::move(descriptorSetLayout)) {
        static constexpr VkVertexInputBindingDescription bindingDescription = getBindingDescription<VertexType>();
        static constexpr auto attributeDescriptions = getAttributeDescriptions<VertexType>();
        _vertexInputInfo = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDescription,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data()
        };
    }
	const VkPipelineVertexInputStateCreateInfo& getVkPipelineVertexInputStateCreateInfo() const {
        return _vertexInputInfo;
	}
};

enum class ShaderProgramType {
    PBR,
    PBR_TESSELLATION,
    SKYBOX,
    SHADOW,
    SKYBOX_OFFSCREEN,
    PBR_OFFSCREEN
};

class ShaderProgramFactory {
public:
    static std::unique_ptr<GraphicsShaderProgram> createShaderProgram(ShaderProgramType type, const LogicalDevice& logicalDevice) {
        switch (type) {
        case ShaderProgramType::PBR:
            return configurePBRProgram(logicalDevice);
            break;
        case ShaderProgramType::PBR_TESSELLATION:
            return configurePBRTesselationProgram(logicalDevice);
            break;
        case ShaderProgramType::SKYBOX:
            return configureSkyboxProgram(logicalDevice);
            break;
        case ShaderProgramType::SHADOW:
            return configureShadowProgram(logicalDevice);
            break;
        case ShaderProgramType::SKYBOX_OFFSCREEN:
            return configureSkyboxOffscreenProgram(logicalDevice);
            break;
        case ShaderProgramType::PBR_OFFSCREEN:
            return configurePBROffscreenProgram(logicalDevice);
            break;
        default:
            throw std::invalid_argument("Unsupported ShaderProgramType");
        }
    }

private:
    static std::unique_ptr<GraphicsShaderProgram> configurePBRProgram(const LogicalDevice& logicalDevice);
    static std::unique_ptr<GraphicsShaderProgram> configurePBRTesselationProgram(const LogicalDevice& logicalDevice);
    static std::unique_ptr<GraphicsShaderProgram> configureSkyboxProgram(const LogicalDevice& logicalDevice);
    static std::unique_ptr<GraphicsShaderProgram> configureShadowProgram(const LogicalDevice& logicalDevice);
    static std::unique_ptr<GraphicsShaderProgram> configureSkyboxOffscreenProgram(const LogicalDevice& logicalDevice);
    static std::unique_ptr<GraphicsShaderProgram> configurePBROffscreenProgram(const LogicalDevice& logicalDevice);
};