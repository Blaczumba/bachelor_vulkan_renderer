#include "pipeline.h"

#include <algorithm>
#include <iterator>

#include "vulkan/wrapper/util/check.h"

PipelineRAII::PipelineRAII(
    const LogicalDevice& logicalDevice, VkPipeline pipeline,
                           VkPipelineBindPoint bindPoint, VkPipelineLayout layout)
  : _logicalDevice(&logicalDevice), _pipeline(pipeline), _bindPoint(bindPoint), _layout(layout) {}

ErrorOr<PipelineRAII> PipelineRAII::create(
    const LogicalDevice& logicalDevice,
    const VkGraphicsPipelineCreateInfo& createInfo) {
  VkPipeline pipeline;
  CHECK_VKCMD(vkCreateGraphicsPipelines(logicalDevice.getVkDevice(), VK_NULL_HANDLE, 1,
                            &createInfo, nullptr, &pipeline));
  return PipelineRAII(logicalDevice, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS, createInfo.layout);
}

std::vector<ErrorOr<PipelineRAII>> PipelineRAII::create(
    const LogicalDevice& logicalDevice,
    std::span<const VkGraphicsPipelineCreateInfo> createInfos) {
  lib::Buffer<VkPipeline> vkPipelines(createInfos.size());
  const VkResult result = vkCreateGraphicsPipelines(
      logicalDevice.getVkDevice(), VK_NULL_HANDLE, static_cast<uint32_t>(createInfos.size()), createInfos.data(), nullptr,
      vkPipelines.data());
  std::vector<ErrorOr<PipelineRAII>> pipelines;
  pipelines.reserve(vkPipelines.size());
  //std::transform(
  //    vkPipelines.cbegin(), vkPipelines.cend(), std::back_inserter(pipelines),
  //    [result, &logicalDevice](VkPipeline pipeline) -> ErrorOr<PipelineRAII> {
  //      if (pipeline != VK_NULL_HANDLE) [[likely]] {
  //        return PipelineRAII(logicalDevice, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS);
  //      }
  //      return Error(result);
  //    });
  return pipelines;
}

PipelineRAII::PipelineRAII(PipelineRAII&& other) noexcept : _logicalDevice(std::exchange(other._logicalDevice, nullptr)), _pipeline(std::exchange(other._pipeline, VK_NULL_HANDLE)), _bindPoint(other._bindPoint), _layout(std::exchange(other._layout, VK_NULL_HANDLE)) {}

PipelineRAII& PipelineRAII::operator=(PipelineRAII&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  _logicalDevice = std::exchange(other._logicalDevice, nullptr);
  _pipeline = std::exchange(other._pipeline, VK_NULL_HANDLE);
  _bindPoint = other._bindPoint;
  _layout = other._layout;

  return *this;
}

PipelineRAII::~PipelineRAII() {
  if (_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(_logicalDevice->getVkDevice(), _pipeline, nullptr);
  }
}

VkPipeline PipelineRAII::getVkPipeline() const {
  return _pipeline;
}

VkPipelineBindPoint PipelineRAII::getVkPipelineBindPoint() const {
  return _bindPoint;
}

VkPipelineLayout PipelineRAII::getVkPipelineLayout() const {
  return _layout;
}

Pipeline::Pipeline(VkPipelineBindPoint pipelineBindPoint) : _pipelineBindPoint(pipelineBindPoint) {}

VkPipeline Pipeline::getVkPipeline() const {
  return _pipeline;
}

VkPipelineLayout Pipeline::getVkPipelineLayout() const {
  return _pipelineLayout;
}

VkPipelineBindPoint Pipeline::getVkPipelineBindPoint() const {
  return _pipelineBindPoint;
}
