#pragma once

#include <unordered_map>
#include <string_view>

#include "common/status/status.h"

#include "vulkan/wrapper/pipeline/shader.h"
#include "common/file/file_loader.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"
#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"

enum class RAIIDescriptorSetType : uint8_t {
  BINDLESS,
  CAMERA
};

class PipelineManager {
public:
  PipelineManager(const std::shared_ptr<FileLoader>& fileLoader);

  ~PipelineManager() = default;

  const Shader* getShader(std::string_view shaderPath) const;

  VkDescriptorSetLayout getVkDescriptorSetLayout(DescriptorSetType type) const;

  ErrorOr<GraphicsPipelineBuilder> createPBRProgram(const Renderpass& renderpass);

  ErrorOr<GraphicsPipelineBuilder> createPbrEnvMappingProgram(const Renderpass& renderpass);

  ErrorOr<GraphicsPipelineBuilder> createSkyboxProgram(const Renderpass& renderpass);

  ErrorOr<GraphicsPipelineBuilder> createShadowProgram(const Renderpass& renderpass);

private:
  std::shared_ptr<FileLoader> _fileLoader;

  std::unordered_map<std::string_view, Shader> _shaders;
  std::unordered_map<RAIIDescriptorSetType, DescriptorSetLayout> _descriptorSetLayouts;
  std::unordered_map<std::string_view, PipelineLayout> _pipelineLayouts;

  ErrorOr<std::reference_wrapper<const Shader>> addShader(const LogicalDevice& logicalDevice, std::string_view shaderFile,
                   VkShaderStageFlagBits shaderStages);

  ErrorOr<VkDescriptorSetLayout> getOrCreateBindlessLayout(const LogicalDevice& logicalDevice);

  ErrorOr<VkDescriptorSetLayout> getOrCreateCameraLayout(const LogicalDevice& logicalDevice);

  ErrorOr<std::reference_wrapper<const PipelineLayout>> getOrCreatePipelineLayout(
      std::string_view id, const LogicalDevice& logicalDevice,
      std::span<const VkDescriptorSetLayout> descriptorSetLayouts = {},
      std::span<const VkPushConstantRange> pushConstantRanges = {},
      VkPipelineLayoutCreateFlags flags = 0);
};
