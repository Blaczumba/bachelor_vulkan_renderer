#pragma once

#include <string_view>
#include <unordered_map>

#include "common/file/file_loader.h"
#include "lib/sparse/sparse_map.h"
#include "lib/types/util.h"
#include "vulkan/resource_manager/hasher.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"
#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"
#include "vulkan/wrapper/pipeline/shader.h"

enum class DescriptorSetType : uint8_t {
  BINDLESS,
  CAMERA
};

class PipelineManager {
public:
  PipelineManager(const std::shared_ptr<FileLoader>& fileLoader);

  ~PipelineManager() = default;

  const Shader* getShader(std::string_view shaderPath) const;

  VkDescriptorSetLayout getOrCreateBindlessLayout(const LogicalDevice& logicalDevice);

  VkDescriptorSetLayout getOrCreateCameraLayout(const LogicalDevice& logicalDevice);

  GraphicsPipelineBuilder createPBRProgram(const Renderpass& renderpass);

  GraphicsPipelineBuilder createPbrEnvMappingProgram(const Renderpass& renderpass);

  GraphicsPipelineBuilder createSkyboxProgram(const Renderpass& renderpass);

  GraphicsPipelineBuilder createShadowProgram(const Renderpass& renderpass);

private:
  static constexpr uint8_t MAX_PIPELINE_LAYOUTS_PER_POOL = 7;  // 7 PipelineLayouts fit in 128
                                                               // bytes.
  using PipelineLayoutMap = lib::SparseMap<PipelineLayout, MAX_PIPELINE_LAYOUTS_PER_POOL>;
  using PipelineLayoutMapIndex = typename PipelineLayoutMap::IndexType;

  struct PipelineLayoutID {
    std::shared_ptr<PipelineLayoutMap> pipelineLayouts;
    PipelineLayoutMapIndex index;
  };

  std::shared_ptr<FileLoader> _fileLoader;

  std::unordered_map<std::string_view, Shader> _shaders;
  std::unordered_map<DescriptorSetType, DescriptorSetLayout> _descriptorSetLayouts;

  std::unordered_map<PipelineLayoutKey, PipelineLayoutID, PipelineLayoutHasher> _pipelineLayouts;
  std::vector<std::weak_ptr<PipelineLayoutMap>> _freePipelineLayoutPools;

  std::reference_wrapper<const Shader> addShader(
      const LogicalDevice& logicalDevice, std::string_view shaderFile,
      VkShaderStageFlagBits shaderStages);

  std::reference_wrapper<const PipelineLayout> getOrCreatePipelineLayout(
      const PipelineLayoutKey& key, const LogicalDevice& logicalDevice);
};
