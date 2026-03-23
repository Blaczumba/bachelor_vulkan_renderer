#pragma once

#include <map>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

#include "vulkan/wrapper/logical_device/logical_device.h"

class Pipeline {
  Pipeline(const LogicalDevice& logicalDevice, VkPipeline pipeline, VkPipelineBindPoint bindPoint,
           VkShaderStageFlags shaderStageFlags, VkPipelineLayout layout) noexcept;

public:
  Pipeline() noexcept = default;

  static Pipeline createGraphicsPipeline(
      const LogicalDevice& logicalDevice, const VkGraphicsPipelineCreateInfo& createInfo,
      VkShaderStageFlags pushConstantShaderStages);

  static std::vector<Pipeline> createGraphicsPipelines(
      const LogicalDevice& logicalDevice,
      std::span<const VkGraphicsPipelineCreateInfo> createInfos);

  static Pipeline createComputePipeline(
      const LogicalDevice& logicalDevice, const VkComputePipelineCreateInfo& createInfo);

  // TODO: Create with other types of create infos.

  Pipeline(Pipeline&& other) noexcept;

  Pipeline& operator=(Pipeline&& other) noexcept;

  ~Pipeline();

  VkPipeline getVkPipeline() const noexcept;

  VkPipelineBindPoint getVkPipelineBindPoint() const noexcept;

  VkShaderStageFlags getPushConstantVkShaderStageFlags() const noexcept;

  VkPipelineLayout getVkPipelineLayout() const noexcept;

  bool isValid() const noexcept;

private:
  void destroy();

  VkPipeline _pipeline = VK_NULL_HANDLE;
  VkPipelineBindPoint _bindPoint;
  VkShaderStageFlags _pushConstantShaderStages;

  VkPipelineLayout _layout = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice = nullptr;
};

struct SpecializationData {
  void* data;
  size_t dataSize;
  std::map<VkShaderStageFlagBits, std::vector<VkSpecializationMapEntry>> mapEntries;
};
