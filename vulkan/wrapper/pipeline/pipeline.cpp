#include "pipeline.h"

#include <algorithm>
#include <iterator>

#include "vulkan/wrapper/util/check.h"

Pipeline::Pipeline(
    const LogicalDevice& logicalDevice, VkPipeline pipeline, VkPipelineBindPoint bindPoint,
    VkShaderStageFlags pushConstantShaderStages, VkPipelineLayout layout) noexcept
  : _logicalDevice(&logicalDevice), _pipeline(pipeline), _bindPoint(bindPoint),
    _pushConstantShaderStages(pushConstantShaderStages), _layout(layout) {}

Pipeline Pipeline::create(
    const LogicalDevice& logicalDevice, const VkGraphicsPipelineCreateInfo& createInfo,
    VkShaderStageFlags shaderStageFlags) {
  VkPipeline pipeline;
  CHECK_VKCMD(vkCreateGraphicsPipelines(
                  logicalDevice.getVkDevice(), VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline),
              "Failed to create VkPipeline.");
  return Pipeline(logicalDevice, pipeline, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderStageFlags,
                  createInfo.layout);
}

std::vector<Pipeline> Pipeline::create(
    const LogicalDevice& logicalDevice, std::span<const VkGraphicsPipelineCreateInfo> createInfos) {
  lib::Buffer<VkPipeline> vkPipelines(createInfos.size());
  vkCreateGraphicsPipelines(
      logicalDevice.getVkDevice(), VK_NULL_HANDLE, static_cast<uint32_t>(createInfos.size()),
      createInfos.data(), nullptr, vkPipelines.data());
  std::vector<Pipeline> pipelines;
  pipelines.reserve(vkPipelines.size());
  std::transform(
      vkPipelines.cbegin(), vkPipelines.cend(), std::cbegin(createInfos),
      std::back_inserter(pipelines),
      [&logicalDevice](VkPipeline vkPipeline, const VkGraphicsPipelineCreateInfo& createInfo) {
        // TODO: Pass correct shader stage flags.
        return vkPipeline != VK_NULL_HANDLE ?
                   Pipeline(logicalDevice, vkPipeline, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            VK_SHADER_STAGE_ALL, createInfo.layout) :
                   Pipeline{};
      });
  return pipelines;
}

Pipeline::Pipeline(Pipeline&& other) noexcept
  : _logicalDevice(std::exchange(other._logicalDevice, nullptr)),
    _pipeline(std::exchange(other._pipeline, VK_NULL_HANDLE)), _bindPoint(other._bindPoint),
    _pushConstantShaderStages(other._pushConstantShaderStages),
    _layout(std::exchange(other._layout, VK_NULL_HANDLE)) {}

void Pipeline::destroy() {
  if (_pipeline != VK_NULL_HANDLE) {
    _logicalDevice->destroyResource([pipeline = _pipeline](DestroyerContext context) {
      vkDestroyPipeline(context.device, pipeline, context.allocationCallbacks);
    });
  }
}

Pipeline& Pipeline::operator=(Pipeline&& other) noexcept {
  if (this == &other) {
    return *this;
  }

  destroy();

  _logicalDevice = std::exchange(other._logicalDevice, nullptr);
  _pipeline = std::exchange(other._pipeline, VK_NULL_HANDLE);
  _bindPoint = other._bindPoint;
  _pushConstantShaderStages = other._pushConstantShaderStages;
  _layout = std::exchange(other._layout, VK_NULL_HANDLE);

  return *this;
}

Pipeline::~Pipeline() {
  destroy();
}

VkPipeline Pipeline::getVkPipeline() const noexcept {
  return _pipeline;
}

VkPipelineBindPoint Pipeline::getVkPipelineBindPoint() const noexcept {
  return _bindPoint;
}

VkShaderStageFlags Pipeline::getPushConstantVkShaderStageFlags() const noexcept {
  return _pushConstantShaderStages;
}

VkPipelineLayout Pipeline::getVkPipelineLayout() const noexcept {
  return _layout;
}

bool Pipeline::isValid() const noexcept {
  return _pipeline != VK_NULL_HANDLE;
}
