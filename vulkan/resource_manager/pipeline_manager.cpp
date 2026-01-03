#include "pipeline_manager.h"

#include <array>
#include <filesystem>
#include <numeric>
#include <string_view>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"
#include "vulkan/wrapper/pipeline/input_description.h"

PipelineManager::PipelineManager(const FileLoader& fileLoader)
  : _fileLoader(fileLoader), _freePipelineLayoutIndices(MAX_PIPELINE_LAYOUTS),
    _freePipelineIndices(MAX_PIPELINES) {
  std::iota(_freePipelineLayoutIndices.rbegin(), _freePipelineLayoutIndices.rend(), 0);
  std::iota(_freePipelineIndices.rbegin(), _freePipelineIndices.rend(), 0);
}

std::unique_ptr<PipelineManager> PipelineManager::create(const FileLoader& fileLoader) {
  return std::unique_ptr<PipelineManager>(new PipelineManager(fileLoader));
}

std::reference_wrapper<const Shader> PipelineManager::addShader(
    const LogicalDevice& logicalDevice, std::string_view shaderFile,
    VkShaderStageFlagBits shaderStages) {
  auto [it, inserted] = _shaders.try_emplace(shaderFile);
  if (!inserted) {
    return it->second;
  }

  const lib::Buffer<std::byte> shaderData =
      _fileLoader.loadFileToBuffer((std::filesystem::path(SHADERS_PATH) / shaderFile).string());
  return it->second = Shader::create(logicalDevice, shaderData, shaderStages);
}

VkDescriptorSetLayout PipelineManager::getOrCreateBindlessLayout(
    const LogicalDevice& logicalDevice) {
  static constexpr DescriptorSetType layoutType = DescriptorSetType::BINDLESS;
  if (auto it = _descriptorSetLayouts.find(layoutType); it != _descriptorSetLayouts.cend()) {
    return it->second.getVkDescriptorSetLayout();
  }

  static constexpr VkDescriptorSetLayoutBinding bindings[] = {
    {
     .binding = 0,
     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
     .descriptorCount = 200,
     .stageFlags = VK_SHADER_STAGE_ALL,
     },
    {
     .binding = 1,
     .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
     .descriptorCount = 200,
     .stageFlags = VK_SHADER_STAGE_ALL,
     }
  };

  static constexpr VkDescriptorBindingFlags flags{
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT};
  static constexpr VkDescriptorBindingFlags bindingFlags[] = {flags, flags};

  DescriptorSetLayout layout = DescriptorSetLayout::create(
      logicalDevice, bindings, bindingFlags,
      VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT);
  const VkDescriptorSetLayout vkLayout = layout.getVkDescriptorSetLayout();
  _descriptorSetLayouts.emplace(layoutType, std::move(layout));
  return vkLayout;
}

VkDescriptorSetLayout PipelineManager::getOrCreateCameraLayout(const LogicalDevice& logicalDevice) {
  static constexpr DescriptorSetType layoutType = DescriptorSetType::CAMERA;
  if (auto it = _descriptorSetLayouts.find(layoutType); it != _descriptorSetLayouts.cend()) {
    return it->second.getVkDescriptorSetLayout();
  }

  static constexpr VkDescriptorSetLayoutBinding bindings[] = {
    {
     .binding = 0,
     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
     .descriptorCount = 1,
     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
     },
  };

  DescriptorSetLayout layout = DescriptorSetLayout::create(logicalDevice, bindings);
  const VkDescriptorSetLayout vkLayout = layout.getVkDescriptorSetLayout();
  _descriptorSetLayouts.emplace(layoutType, std::move(layout));
  return vkLayout;
}

std::pair<PipelineLayout*, PipelineManager::PipelineLayoutMapIndex> PipelineManager::
    getOrCreatePipelineLayout(const PipelineLayoutKey& key, const LogicalDevice& logicalDevice) {
  auto [it, inserted] = _pipelineLayoutIndices.try_emplace(key);

  if (!inserted) {
    PipelineLayoutID& layoutID = _pipelineLayouts.getValue(it->second);
    layoutID.refCount++;
    return {&layoutID.layout, it->second};
  }

  const PipelineLayoutMapIndex index = _freePipelineLayoutIndices.back();
  _freePipelineLayoutIndices.pop_back();
  it->second = index;
  _pipelineLayoutKeys.emplace(index, key);
  return {
    &_pipelineLayouts
         .insertUnsafe(
             index, PipelineLayoutID{PipelineLayout::create(logicalDevice, key.descriptorSetLayouts,
                                                            key.pushConstants, key.createFlags),
                                     1})
         .layout,
    index};
}

bool PipelineManager::removePipelineLayout(PipelineLayoutMapIndex index) {
  if (!_pipelineLayouts.exists(index)) {
    return false;
  }

  PipelineLayoutID& layoutID = _pipelineLayouts.getValue(index);
  if (--layoutID.refCount > 0) {
    return true;
  }

  _pipelineLayouts.eraseUnsafe(index);
  _freePipelineLayoutIndices.push_back(index);
  auto it = _pipelineLayoutKeys.find(index);
  _pipelineLayoutIndices.erase(it->second);
  _pipelineLayoutKeys.erase(it);
  return true;
}

Pipeline* PipelineManager::getPipeline(PipelineMapIndex index) {
  if (!_pipelines.exists(index)) {
    return nullptr;
  }

  return &_pipelines.getValue(index).pipeline;
}

bool PipelineManager::removePipeline(PipelineManager::PipelineMapIndex index) {
  if (!_pipelines.exists(index)) {
    return false;
  }

  const PipelineLayoutMapIndex layoutIndex = _pipelines.getValue(index).layoutIndex;
  _pipelines.eraseUnsafe(index);
  _freePipelineIndices.push_back(index);
  return removePipelineLayout(layoutIndex);
}

namespace {

template <typename T>
constexpr VkPushConstantRange getPushConstantRange(
    VkShaderStageFlags shaderStages, uint32_t offset = 0) {
  return VkPushConstantRange{.stageFlags = shaderStages, .offset = offset, .size = sizeof(T)};
}

}  // namespace

PipelineManager::PipelineMapIndex PipelineManager::createPBRProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex =
      addShader(logicalDevice, "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const auto [pipelineLayout, pipelineLayoutIndex] = getOrCreatePipelineLayout(
      PipelineLayoutKey{
        {getOrCreateBindlessLayout(logicalDevice), getOrCreateCameraLayout(logicalDevice)},
        {getPushConstantRange<PushConstantsModelDescriptorHandles>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}
  },
      logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexPTNT>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexPTNT>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  const PipelineMapIndex pipelineIndex = _freePipelineIndices.back();
  _freePipelineIndices.pop_back();
  _pipelines.insertUnsafe(
      pipelineIndex,
      PipelineResource{
        GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
            .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
            .withShaderStageCreateInfo(shaderStages)
            .withViewportStateCreateInfo()
            .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT)
            .withMultisampleStateCreateInfo(
                renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
            .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
            .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)
            .createPipeline(renderpass, *pipelineLayout),
        pipelineLayoutIndex});
  return pipelineIndex;
}

PipelineManager::PipelineMapIndex PipelineManager::createPbrEnvMappingProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex =
      addShader(logicalDevice, "pbr_env_mapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const auto [pipelineLayout, pipelineLayoutIndex] = getOrCreatePipelineLayout(
      PipelineLayoutKey{{getOrCreateBindlessLayout(logicalDevice)},
                        {getPushConstantRange<PushConstantsModelDescriptorHandles>(
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}},
      logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexPTNT>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexPTNT>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  const PipelineMapIndex pipelineIndex = _freePipelineIndices.back();
  _freePipelineIndices.pop_back();
  _pipelines.insertUnsafe(
      pipelineIndex,
      PipelineResource{
        GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
            .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
            .withShaderStageCreateInfo(shaderStages)
            .withViewportStateCreateInfo()
            .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT)
            .withMultisampleStateCreateInfo(
                renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
            .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
            .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)
            .createPipeline(renderpass, *pipelineLayout),
        pipelineLayoutIndex});
  return pipelineIndex;
}

PipelineManager::PipelineMapIndex PipelineManager::createEnvMappingProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex =
      addShader(logicalDevice, "env_mapping_phong.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "env_mapping_phong.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const auto [pipelineLayout, pipelineLayoutIndex] = getOrCreatePipelineLayout(
      PipelineLayoutKey{
        {getOrCreateBindlessLayout(logicalDevice), getOrCreateCameraLayout(logicalDevice)},
        {getPushConstantRange<PushConstantsModelDescriptorHandles>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}
  },
      logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexPN>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexPN>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  const PipelineMapIndex pipelineIndex = _freePipelineIndices.back();
  _freePipelineIndices.pop_back();
  _pipelines.insertUnsafe(
      pipelineIndex,
      PipelineResource{
        GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
            .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
            .withShaderStageCreateInfo(shaderStages)
            .withViewportStateCreateInfo()
            .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT)
            .withMultisampleStateCreateInfo(
                renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
            .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
            .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)
            .createPipeline(renderpass, *pipelineLayout),
        pipelineLayoutIndex});
  return pipelineIndex;
}

PipelineManager::PipelineMapIndex PipelineManager::createSkyboxProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex = addShader(logicalDevice, "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const auto [pipelineLayout, pipelineLayoutIndex] = getOrCreatePipelineLayout(
      PipelineLayoutKey{{getOrCreateBindlessLayout(logicalDevice)},
                        {getPushConstantRange<PushConstantsSkybox>(
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}},
      logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
                          | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexP>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexP>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  const PipelineMapIndex pipelineIndex = _freePipelineIndices.back();
  _freePipelineIndices.pop_back();
  _pipelines.insertUnsafe(
      pipelineIndex,
      PipelineResource{
        GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
            .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
            .withShaderStageCreateInfo(shaderStages)
            .withViewportStateCreateInfo()
            .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT)
            .withMultisampleStateCreateInfo(
                renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
            .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
            .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)
            .createPipeline(renderpass, *pipelineLayout),
        pipelineLayoutIndex});
  return pipelineIndex;
}

PipelineManager::PipelineMapIndex PipelineManager::createShadowProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex = addShader(logicalDevice, "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const auto [pipelineLayout, pipelineLayoutIndex] = getOrCreatePipelineLayout(
      PipelineLayoutKey{
        {}, {getPushConstantRange<PushConstantsShadow>(VK_SHADER_STAGE_VERTEX_BIT)}},
      logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{.colorWriteMask = VK_COLOR_COMPONENT_R_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexP>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexP>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  const PipelineMapIndex pipelineIndex = _freePipelineIndices.back();
  _freePipelineIndices.pop_back();
  _pipelines.insertUnsafe(
      pipelineIndex,
      PipelineResource{
        GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
            .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
            .withShaderStageCreateInfo(shaderStages)
            .withViewportStateCreateInfo()
            .withRasterizationStateCreateInfo(
                VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, std::pair{0.7f, 2.0f})
            .withMultisampleStateCreateInfo(renderpass.getAttachmentsLayout().getNumMsaaSamples())
            .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
            .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)
            .createPipeline(renderpass, *pipelineLayout),
        pipelineLayoutIndex});
  return pipelineIndex;
}
