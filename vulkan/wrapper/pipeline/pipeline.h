#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/wrapper/logical_device/logical_device.h>

class Pipeline {
  Pipeline(const LogicalDevice& logicalDevice, VkPipeline pipeline, VkPipelineBindPoint bindPoint,
           VkPipelineLayout layout) noexcept;

public:
  Pipeline() noexcept = default;

  static Pipeline create(
      const LogicalDevice& logicalDevice, const VkGraphicsPipelineCreateInfo& createInfo);

  static std::vector<Pipeline> create(const LogicalDevice& logicalDevice,
                                      std::span<const VkGraphicsPipelineCreateInfo> createInfos);

  // TODO: Create with other types of create infos.

  Pipeline(Pipeline&& other) noexcept;

  Pipeline& operator=(Pipeline&& other) noexcept;

  ~Pipeline();

  VkPipeline getVkPipeline() const;

  VkPipelineBindPoint getVkPipelineBindPoint() const;

  VkPipelineLayout getVkPipelineLayout() const;

  bool isValid() const;

private:
  void destroy();

  VkPipeline _pipeline = VK_NULL_HANDLE;
  VkPipelineBindPoint _bindPoint;

  VkPipelineLayout _layout = VK_NULL_HANDLE;

  const LogicalDevice* _logicalDevice = nullptr;
};
