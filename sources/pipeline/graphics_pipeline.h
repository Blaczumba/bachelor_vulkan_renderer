#pragma once

#include "pipeline.h"
#include "render_pass/render_pass.h"
#include "primitives/vk_primitives.h"
#include "memory_objects/uniform_buffer/push_constants.h"
#include "shader/shader.h"

#include <vulkan/vulkan.h>

#include <unordered_map>
#include <algorithm>
#include <string>
#include <stdexcept>

struct GraphicsPipelineParameters {
    VkCullModeFlags cullMode            = VK_CULL_MODE_BACK_BIT;
    VkSampleCountFlagBits msaaSamples   = VK_SAMPLE_COUNT_1_BIT;
    float depthBiasConstantFactor       = 0.0f;
    float depthBiasSlopeFactor          = 0.0f;
};

class GraphicsPipeline : public Pipeline {
    std::unordered_map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo> _shaderInfos;
    VkDescriptorSetLayout _descriptorSetLayout = VK_NULL_HANDLE;
    VkVertexInputBindingDescription _bindingDescription;
    std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;
    PushConstants _pushConstants;
    GraphicsPipelineParameters _parameters;

    const Renderpass& _renderpass;

public:
	GraphicsPipeline(const LogicalDevice& logicalDevice, const Renderpass& renderpass);
	~GraphicsPipeline();
    void create();
    void cleanup();

    void setShader(const Shader& shader);
    void setDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
    void setPushConstants(const PushConstants& pushConstants);
    void setPipelineParameters(const GraphicsPipelineParameters& parameters);
    template<typename VertexType>
    void setVertexDescriptions();
};

template<typename VertexType>
void GraphicsPipeline::setVertexDescriptions() {
    _bindingDescription = getBindingDescription<VertexType>();
    _attributeDescriptions = getAttributeDescriptions<VertexType>();
}
