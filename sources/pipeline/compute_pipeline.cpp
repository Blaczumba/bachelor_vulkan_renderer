#include "compute_pipeline.h"
#include "shader_utils.h"
#include "shader/shader.h"

#include <stdexcept>

ComputePipeline::ComputePipeline(std::shared_ptr<LogicalDevice> logicalDevice, VkDescriptorSetLayout descriptorSetLayout, const std::string& computeShader)
    : Pipeline(logicalDevice, VK_PIPELINE_BIND_POINT_COMPUTE) {
    const VkDevice device = _logicalDevice->getVkDevice();
    
    const Shader shader(*logicalDevice, "", VK_SHADER_STAGE_COMPUTE_BIT);

    VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
    computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computeShaderStageInfo.stage = shader.getVkShaderStageFlagBits();
    computeShaderStageInfo.module = shader.getVkShaderModule();
    computeShaderStageInfo.pName = shader.getName().data();

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.stage = computeShaderStageInfo;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }
}

ComputePipeline::~ComputePipeline() {
    VkDevice device = _logicalDevice->getVkDevice();

    vkDestroyPipeline(device, _pipeline, nullptr);
    vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
}
