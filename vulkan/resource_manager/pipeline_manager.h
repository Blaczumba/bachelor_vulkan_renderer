#pragma once

#include <glm/glm.hpp>
#include <string_view>
#include <unordered_map>

#include "common/file/file_loader.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/hasher.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"
#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"
#include "vulkan/wrapper/pipeline/shader.h"

enum class DescriptorSetType : uint8_t {
  BINDLESS,
  CAMERA
};

class PipelineManager {
  static constexpr size_t MAX_PIPELINE_LAYOUTS = 32;

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

  PipelineManager(const FileLoader& fileLoader);

public:
  static std::unique_ptr<PipelineManager> create(const FileLoader& fileLoader);

  ~PipelineManager() = default;

  VkDescriptorSetLayout getOrCreateBindlessLayout(const LogicalDevice& logicalDevice);

  VkDescriptorSetLayout getOrCreateCameraLayout(const LogicalDevice& logicalDevice);

  using PipelineMapIndex = typename PipelineMap::IndexType;

  Pipeline* getPipeline(PipelineMapIndex index);

  bool removePipeline(PipelineMapIndex index);

  PipelineMapIndex createPBRProgram(const Renderpass& renderpass);

  PipelineMapIndex createPbrEnvMappingProgram(const Renderpass& renderpass);

  PipelineMapIndex createEnvMappingProgram(const Renderpass& renderpass);

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

  const FileLoader& _fileLoader;

  std::unordered_map<std::string_view, Shader> _shaders;
  std::unordered_map<DescriptorSetType, DescriptorSetLayout> _descriptorSetLayouts;

  const Shader& addShader(const LogicalDevice& logicalDevice, std::string_view shaderFile,
                          VkShaderStageFlagBits shaderStages);

  std::pair<PipelineLayout*, PipelineLayoutMapIndex> getOrCreatePipelineLayout(
      const PipelineLayoutKey& key, const LogicalDevice& logicalDevice);

  bool removePipelineLayout(PipelineLayoutMapIndex index);
};

struct PushConstantsModelDescriptorHandles {
  glm::mat4 model;
  uint16_t descriptorHandles[32];
};

struct PushConstantsShadow {
  glm::mat4 model;
  glm::mat4 lightProjView;
};

struct PushConstantsPhongEnv {
  glm::mat4 lightProjView;
  glm::mat3x4 model;
  uint32_t envMapHandle;
};

struct PushConstantsSkybox {
  glm::mat4 proj;
  glm::mat3x4 view;
  uint32_t skyboxHandle;
};
