#include "compute_pipeline.h"
#include "shader_utils.h"
#include "shader/shader.h"

#include <stdexcept>

ComputePipeline::ComputePipeline(const LogicalDevice& logicalDevice, VkDescriptorSetLayout descriptorSetLayout, const std::string& computeShader)
    : Pipeline(VK_PIPELINE_BIND_POINT_COMPUTE), _logicalDevice(logicalDevice) {
    const VkDevice device = _logicalDevice.getVkDevice();
    
    const Shader shader(logicalDevice, "", VK_SHADER_STAGE_COMPUTE_BIT);

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
    pipelineInfo.stage = shader.getVkPipelineStageCreateInfo();

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }
}

ComputePipeline::~ComputePipeline() {
    VkDevice device = _logicalDevice.getVkDevice();

    vkDestroyPipeline(device, _pipeline, nullptr);
    vkDestroyPipelineLayout(device, _pipelineLayout, nullptr);
}
