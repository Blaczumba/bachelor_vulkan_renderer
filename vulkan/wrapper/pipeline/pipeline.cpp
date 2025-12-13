#include "pipeline.h"

#include <algorithm>
#include <iterator>

#include "vulkan/wrapper/util/check.h"

Pipeline::Pipeline(const LogicalDevice& logicalDevice, VkPipeline pipeline,
                   VkPipelineBindPoint bindPoint, VkPipelineLayout layout)
  : _logicalDevice(&logicalDevice), _pipeline(pipeline), _bindPoint(bindPoint), _layout(layout) {}

ErrorOr<Pipeline> Pipeline::create(
    const LogicalDevice& logicalDevice, const VkGraphicsPipelineCreateInfo& createInfo) {
  VkPipeline pipeline;
  CHECK_VKCMD(vkCreateGraphicsPipelines(
      logicalDevice.getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline));
  return Pipeline(logicalDevice, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS, createInfo.layout);
}

std::vector<ErrorOr<Pipeline>> Pipeline::create(
    const LogicalDevice& logicalDevice, std::span<const VkGraphicsPipelineCreateInfo> createInfos) {
  lib::Buffer<VkPipeline> vkPipelines(createInfos.size());
  const VkResult result = vkCreateGraphicsPipelines(
      logicalDevice.getVkDevice(), VK_NULL_HANDLE, static_cast<uint32_t>(createInfos.size()),
      createInfos.data(), nullptr, vkPipelines.data());
  std::vector<ErrorOr<Pipeline>> pipelines;
  pipelines.reserve(vkPipelines.size());
  // std::transform(
  //     vkPipelines.cbegin(), vkPipelines.cend(), std::back_inserter(pipelines),
  //     [result, &logicalDevice](VkPipeline pipeline) -> ErrorOr<Pipeline> {
  //       if (pipeline != VK_NULL_HANDLE) [[likely]] {
  //         return Pipeline(logicalDevice, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
  //       }
  //       return Error(result);
  //     });
  return pipelines;
}

Pipeline::Pipeline(Pipeline&& other) noexcept
  : _logicalDevice(std::exchange(other._logicalDevice, nullptr)),
    _pipeline(std::exchange(other._pipeline, VK_NULL_HANDLE)), _bindPoint(other._bindPoint),
    _layout(std::exchange(other._layout, VK_NULL_HANDLE)) {}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  _logicalDevice = std::exchange(other._logicalDevice, nullptr);
  _pipeline = std::exchange(other._pipeline, VK_NULL_HANDLE);
  _bindPoint = other._bindPoint;
  _layout = other._layout;

  return *this;
}

Pipeline::~Pipeline() {
  if (_pipeline != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([pipeline = _pipeline](DestroyerContext context) {
      vkDestroyPipeline(context.device, pipeline, context.allocationCallbacks);
    });
  }
}

VkPipeline Pipeline::getVkPipeline() const {
  return _pipeline;
}

VkPipelineBindPoint Pipeline::getVkPipelineBindPoint() const {
  return _bindPoint;
}

VkPipelineLayout Pipeline::getVkPipelineLayout() const {
  return _layout;
}
