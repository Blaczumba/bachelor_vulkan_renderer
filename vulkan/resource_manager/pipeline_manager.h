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
  static constexpr uint8_t MAX_PIPELINE_LAYOUTS = 32;

  struct PipelineLayoutID {
    PipelineLayout layout;
    size_t refCount;
  };

  using PipelineLayoutMap = lib::SparseMap<PipelineLayoutID, MAX_PIPELINE_LAYOUTS>;
  using PipelineLayoutMapIndex = typename PipelineLayoutMap::IndexType;

  static constexpr size_t MAX_PIPELINES = 32;

  struct PipelineResource {
    Pipeline pipeline;
    PipelineLayoutMapIndex layoutIndex;
  };

  using PipelineMap = lib::SparseMap<PipelineResource, MAX_PIPELINES>;

public:
  using PipelineMapIndex = typename PipelineMap::IndexType;

  PipelineManager(const std::shared_ptr<FileLoader>& fileLoader);

  ~PipelineManager() = default;

  const Shader* getShader(std::string_view shaderPath) const;

  VkDescriptorSetLayout getOrCreateBindlessLayout(const LogicalDevice& logicalDevice);

  VkDescriptorSetLayout getOrCreateCameraLayout(const LogicalDevice& logicalDevice);

  Pipeline* getPipeline(PipelineMapIndex index);

  bool removePipeline(PipelineMapIndex index);

  PipelineMapIndex createPBRProgram(const Renderpass& renderpass);

  PipelineMapIndex createPbrEnvMappingProgram(const Renderpass& renderpass);

  PipelineMapIndex createSkyboxProgram(const Renderpass& renderpass);

  PipelineMapIndex createShadowProgram(const Renderpass& renderpass);

private:
  PipelineLayoutMap _pipelineLayouts;
  std::vector<PipelineLayoutMapIndex> _freePipelineLayoutIndices;

  std::unordered_map<PipelineLayoutKey, PipelineLayoutMapIndex, PipelineLayoutHasher>
      _pipelineLayoutIndices;
  std::unordered_map<PipelineLayoutMapIndex, PipelineLayoutKey> _pipelineLayoutKeys;

  PipelineMap _pipelines;
  std::vector<PipelineMapIndex> _freePipelineIndices;

  std::shared_ptr<FileLoader> _fileLoader;

  std::unordered_map<std::string_view, Shader> _shaders;
  std::unordered_map<DescriptorSetType, DescriptorSetLayout> _descriptorSetLayouts;

  std::reference_wrapper<const Shader> addShader(
      const LogicalDevice& logicalDevice, std::string_view shaderFile,
      VkShaderStageFlagBits shaderStages);

  std::pair<PipelineLayout*, PipelineLayoutMapIndex> getOrCreatePipelineLayout(
      const PipelineLayoutKey& key, const LogicalDevice& logicalDevice);

  bool removePipelineLayout(PipelineLayoutMapIndex index);
};
