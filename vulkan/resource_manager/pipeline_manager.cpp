#include "pipeline_manager.h"

#include <array>
#include <string_view>
#include <vulkan/vulkan.h>
#include <filesystem>

#include "lib/buffer/buffer.h"

#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"
#include "vulkan/wrapper/pipeline/input_description.h"

PipelineManager::PipelineManager(const std::shared_ptr<FileLoader>& fileLoader) : _fileLoader(fileLoader) {}

ErrorOr<std::reference_wrapper<const Shader>> PipelineManager::addShader(
    const LogicalDevice& logicalDevice, std::string_view shaderFile,
    VkShaderStageFlagBits shaderStages) {
  if (auto it = _shaders.find(shaderFile); it != _shaders.cend()) {
    return it->second;
  }

  ASSIGN_OR_RETURN(
      const lib::Buffer<std::byte> shaderData,
      _fileLoader->loadFileToBuffer((std::filesystem::path(SHADERS_PATH) / shaderFile).string()));
  ASSIGN_OR_RETURN(Shader shader, Shader::create(logicalDevice, shaderData, shaderStages));
  const auto[it, _] = _shaders.emplace(shaderFile, std::move(shader));
  return it->second;
}

ErrorOr<VkDescriptorSetLayout> PipelineManager::getOrCreateBindlessLayout(
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

  ASSIGN_OR_RETURN(
      DescriptorSetLayout layout,
      DescriptorSetLayout::create(logicalDevice, bindings, bindingFlags,
                                  VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT));
  const VkDescriptorSetLayout vkLayout = layout.getVkDescriptorSetLayout();
  _descriptorSetLayouts.emplace(layoutType, std::move(layout));
  return vkLayout;
}

ErrorOr<VkDescriptorSetLayout> PipelineManager::getOrCreateCameraLayout(
    const LogicalDevice& logicalDevice) {
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
  ASSIGN_OR_RETURN(
      DescriptorSetLayout layout, DescriptorSetLayout::create(logicalDevice, bindings));
  const VkDescriptorSetLayout vkLayout = layout.getVkDescriptorSetLayout();
  _descriptorSetLayouts.emplace(layoutType, std::move(layout));
  return vkLayout;
}

ErrorOr<std::reference_wrapper<const PipelineLayout>> PipelineManager::getOrCreatePipelineLayout(
    std::string_view id, const LogicalDevice& logicalDevice,
    std::span<const VkDescriptorSetLayout> descriptorSetLayouts,
    std::span<const VkPushConstantRange> pushConstantRanges,
    VkPipelineLayoutCreateFlags flags) {
  auto [it, inserted] = _pipelineLayouts.try_emplace(id);

  if (!inserted) {
    return it->second;
  }

  ASSIGN_OR_RETURN(
      PipelineLayout layout,
      PipelineLayout::create(logicalDevice, descriptorSetLayouts, pushConstantRanges, flags));

  it->second = std::move(layout);
  return it->second;
}

namespace {

template <typename T>
constexpr VkPushConstantRange getPushConstantRange(
    VkShaderStageFlags shaderStages, uint32_t offset = 0) {
  return VkPushConstantRange{.stageFlags = shaderStages, .offset = offset, .size = sizeof(T)};
}

}  // namespace

ErrorOr<GraphicsPipelineBuilder> PipelineManager::createPBRProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  ASSIGN_OR_RETURN(
      const Shader& vertex, addShader(logicalDevice, "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
  ASSIGN_OR_RETURN(const Shader& fragment,
                   addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

  static constexpr VkPushConstantRange pushConstantRanges[] = {
    getPushConstantRange<PushConstantsPBR>(
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)};

  VkDescriptorSetLayout descriptorSetLayouts[2];
  ASSIGN_OR_RETURN(descriptorSetLayouts[0], getOrCreateBindlessLayout(logicalDevice));
  ASSIGN_OR_RETURN(descriptorSetLayouts[1], getOrCreateCameraLayout(logicalDevice));

  ASSIGN_OR_RETURN(const PipelineLayout& pipelineLayout,
                   getOrCreatePipelineLayout("PbrBindlessCam", logicalDevice,
                                             descriptorSetLayouts, pushConstantRanges));

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

  return ErrorOr<GraphicsPipelineBuilder>(
      std::move(GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
      .withRenderpass(renderpass)
      .withPipelineLayout(pipelineLayout)
      .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
      .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
      .withShaderStageCreateInfo(shaderStages)
      .withViewportStateCreateInfo()
      .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT)
      .withMultisampleStateCreateInfo(renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
      .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
      .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}

ErrorOr<GraphicsPipelineBuilder> PipelineManager::createPbrEnvMappingProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  ASSIGN_OR_RETURN(const Shader& vertex,
                   addShader(logicalDevice, "pbr_env_mapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
  ASSIGN_OR_RETURN(const Shader& fragment,
                   addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

  static constexpr VkPushConstantRange pushConstantRanges[] = {
    getPushConstantRange<PushConstantsPBR>(
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)};

  VkDescriptorSetLayout descriptorSetLayouts[1];
  ASSIGN_OR_RETURN(descriptorSetLayouts[0], getOrCreateBindlessLayout(logicalDevice));

  ASSIGN_OR_RETURN(const PipelineLayout& pipelineLayout,
                   getOrCreatePipelineLayout(
                       "PbrBindlessCam", logicalDevice, descriptorSetLayouts, pushConstantRanges));

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

  return ErrorOr<GraphicsPipelineBuilder>(std::move(
      GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
          .withRenderpass(renderpass)
          .withPipelineLayout(pipelineLayout)
          .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
          .withShaderStageCreateInfo(shaderStages)
          .withViewportStateCreateInfo()
          .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT)
          .withMultisampleStateCreateInfo(
              renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
          .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
          .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}

ErrorOr<GraphicsPipelineBuilder> PipelineManager::createSkyboxProgram(
    const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  ASSIGN_OR_RETURN(const Shader& vertex,
                   addShader(logicalDevice, "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
  ASSIGN_OR_RETURN(const Shader& fragment,
                   addShader(logicalDevice, "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

  VkDescriptorSetLayout descriptorSetLayouts[1];
  ASSIGN_OR_RETURN(descriptorSetLayouts[0], getOrCreateBindlessLayout(logicalDevice));

  static constexpr VkPushConstantRange pushConstantRanges[] = {
    getPushConstantRange<PushConstantsSkybox>(
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)};

  ASSIGN_OR_RETURN(const PipelineLayout& pipelineLayout,
                   getOrCreatePipelineLayout(
                       "Skybox", logicalDevice, descriptorSetLayouts, pushConstantRanges));

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

  return ErrorOr<GraphicsPipelineBuilder>(std::move(
      GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
          .withRenderpass(renderpass)
          .withPipelineLayout(pipelineLayout)
          .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
          .withShaderStageCreateInfo(shaderStages)
          .withViewportStateCreateInfo()
          .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_FRONT_BIT)
          .withMultisampleStateCreateInfo(
              renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
          .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
          .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}

ErrorOr<GraphicsPipelineBuilder> PipelineManager::createShadowProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  ASSIGN_OR_RETURN(const Shader& vertex,
                   addShader(logicalDevice, "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT));
  ASSIGN_OR_RETURN(const Shader& fragment,
                   addShader(logicalDevice, "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT));

  static constexpr VkPushConstantRange pushConstantRanges[] = {
    getPushConstantRange<PushConstantsShadow>(
        VK_SHADER_STAGE_VERTEX_BIT)};

  ASSIGN_OR_RETURN(const PipelineLayout& pipelineLayout,
                   getOrCreatePipelineLayout("Shadow", logicalDevice, {}, pushConstantRanges));

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexP>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexP>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  return ErrorOr<GraphicsPipelineBuilder>(std::move(
      GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
          .withRenderpass(renderpass)
          .withPipelineLayout(pipelineLayout)
          .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
          .withShaderStageCreateInfo(shaderStages)
          .withViewportStateCreateInfo()
          .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, std::pair{0.7f, 2.0f})
          .withMultisampleStateCreateInfo(
              renderpass.getAttachmentsLayout().getNumMsaaSamples())
          .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
          .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}

