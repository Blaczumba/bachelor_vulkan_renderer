#include "vulkan/wrapper/pipeline/compute_pipeline_builder.h"

#include <optional>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"
#include "vulkan/wrapper/pipeline/pipeline_layout.h"

Pipeline ComputePipelineBuilder::createPipeline(const PipelineLayout& pipelineLayout) {
  if (!_shaderStage.has_value()) [[unlikely]] {
    throw EngineException("Cannot create compute pipeline: shader stage create info is not set.");
  }

  const VkComputePipelineCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
    .stage = *_shaderStage,
    .layout = pipelineLayout.getVkPipelineLayout()};
  return Pipeline::createComputePipeline(pipelineLayout.getLogicalDevice(), createInfo);
}

ComputePipelineBuilder& ComputePipelineBuilder::withShaderStageCreateInfo(
    const VkPipelineShaderStageCreateInfo& shaderStage) {
  _shaderStage = shaderStage;
  return *this;
}
