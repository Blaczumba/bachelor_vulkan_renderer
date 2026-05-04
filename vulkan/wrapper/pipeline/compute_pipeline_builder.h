#pragma once

#include <optional>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/pipeline/pipeline.h"
#include "vulkan/wrapper/pipeline/pipeline_layout.h"

class ComputePipelineBuilder {
public:
  Pipeline createPipeline(const PipelineLayout& pipelineLayout);

  ComputePipelineBuilder& withShaderStageCreateInfo(
      const VkPipelineShaderStageCreateInfo& shaderStage);

private:
  std::optional<VkPipelineShaderStageCreateInfo> _shaderStage;
};
