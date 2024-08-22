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

template<typename VertexType>
class GraphicsPipeline : public Pipeline {
    std::unordered_map<VkShaderStageFlagBits, VkPipelineShaderStageCreateInfo> _shaderInfos;

public:
	GraphicsPipeline(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Renderpass> renderpass, VkDescriptorSetLayout descriptorSetLayout, const PushConstants& pushConstants, VkSampleCountFlagBits msaaSamples, const std::string& vertexShader, const std::string& fragmentShader, bool backFace, float depthBiasConstantFactor, float depthBiasSlopeFactor);
	~GraphicsPipeline();

    void addShader(const Shader& shader);
};

template<typename VertexType>
void GraphicsPipeline<VertexType>::addShader(const Shader& shader) {
    if (_shaderInfos.contains(shader.getVkShaderStageFlagBits()))
        throw std::runtime_error("Shader staged attached to graphics pipeline twice!");
    _shaderInfos.insert({ shader.getVkShaderStageFlagBits(), shader.getVkPipelineStageCreateInfo() });
}

template<typename VertexType>
GraphicsPipeline<VertexType>::GraphicsPipeline(std::shared_ptr<LogicalDevice> logicalDevice, std::shared_ptr<Renderpass> renderpass, VkDescriptorSetLayout descriptorSetLayout, const PushConstants& pushConstants, VkSampleCountFlagBits msaaSamples, const std::string& vertexShader, const std::string& fragmentShader, bool backFace, float depthBiasConstantFactor, float depthBiasSlopeFactor)
    : Pipeline(logicalDevice, VK_PIPELINE_BIND_POINT_GRAPHICS) {
    const Shader vertShader(*logicalDevice, SHADERS_PATH + vertexShader, VK_SHADER_STAGE_VERTEX_BIT);
    const Shader fragShader(*logicalDevice, SHADERS_PATH + fragmentShader, VK_SHADER_STAGE_FRAGMENT_BIT);

    VkDevice device = _logicalDevice->getVkDevice();

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertShader.getVkPipelineStageCreateInfo(), fragShader.getVkPipelineStageCreateInfo() };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = getBindingDescription<VertexType>();
    auto attributeDescriptions = getAttributeDescriptions<VertexType>();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // rasterizer.cullMode = (backFace) ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_FRONT_BIT;
    // rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    if (depthBiasConstantFactor != 0.0f && depthBiasSlopeFactor != 0.0f)
        rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = depthBiasConstantFactor;
    rasterizer.depthBiasSlopeFactor = depthBiasSlopeFactor;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = msaaSamples;
    multisampling.sampleShadingEnable = VK_TRUE;
    multisampling.minSampleShading = 0.2f;

    uint32_t colorBlendAttachmentsCount = renderpass->getColorAttachmentsCount();
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(colorBlendAttachmentsCount);
    for (VkPipelineColorBlendAttachmentState& colorBlendAttachment : colorBlendAttachments) {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = colorBlendAttachmentsCount;
    colorBlending.pAttachments = colorBlendAttachments.data();
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    const auto& pushConstantsLayout = pushConstants.getVkPushConstantRange();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    if(!pushConstantsLayout.empty())
       pipelineLayoutInfo.pPushConstantRanges = pushConstantsLayout.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstantsLayout.size());

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineInfo.pStages = shaderStages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = renderpass->getVkRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }
}

template<typename VertexType>
GraphicsPipeline<VertexType>::~GraphicsPipeline() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyPipeline(device, _pipeline, nullptr);
    vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
}

