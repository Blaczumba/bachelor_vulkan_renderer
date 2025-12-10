#pragma once

#include <vulkan/vulkan.h>

#include "common/status/status.h"
#include "vulkan/wrapper/pipeline/pipeline_layout.h"

class Pipeline {
protected:
  VkPipeline _pipeline = VK_NULL_HANDLE;
  VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;
  VkPipelineBindPoint _pipelineBindPoint;

public:
  Pipeline(VkPipelineBindPoint pipelineBindPoint);
  virtual ~Pipeline() = default;

  VkPipeline getVkPipeline() const;
  VkPipelineLayout getVkPipelineLayout() const;
  VkPipelineBindPoint getVkPipelineBindPoint() const;
};


class PipelineRAII {
  PipelineRAII(const LogicalDevice& logicalDevice, VkPipeline pipeline,
               VkPipelineBindPoint bindPoint, VkPipelineLayout layout);

public:
  PipelineRAII() = default;

  static ErrorOr<PipelineRAII> create(
      const LogicalDevice& logicalDevice, const VkGraphicsPipelineCreateInfo& createInfo);

  static std::vector<ErrorOr<PipelineRAII>> create(
      const LogicalDevice& logicalDevice,
      std::span<const VkGraphicsPipelineCreateInfo> createInfos);

  // TODO: Create with other types of create infos.

  PipelineRAII(PipelineRAII&& other) noexcept;

  PipelineRAII& operator=(PipelineRAII&& other) noexcept;

  ~PipelineRAII();

  VkPipeline getVkPipeline() const;

  VkPipelineBindPoint getVkPipelineBindPoint() const;

  VkPipelineLayout getVkPipelineLayout() const;

private:
  VkPipeline _pipeline = VK_NULL_HANDLE;
  VkPipelineBindPoint _bindPoint;

  VkPipelineLayout _layout = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice;
};
