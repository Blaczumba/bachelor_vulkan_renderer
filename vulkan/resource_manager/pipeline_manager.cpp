#include "pipeline_manager.h"

#include <array>
#include <filesystem>
#include <string_view>
#include <vulkan/vulkan.h>

#include "lib/buffer/buffer.h"
#include "vulkan/wrapper/pipeline/graphics_pipeline_builder.h"
#include "vulkan/wrapper/pipeline/input_description.h"

PipelineManager::PipelineManager(const std::shared_ptr<FileLoader>& fileLoader)
  : _fileLoader(fileLoader) {}

std::reference_wrapper<const Shader> PipelineManager::addShader(
    const LogicalDevice& logicalDevice, std::string_view shaderFile,
    VkShaderStageFlagBits shaderStages) {
  auto [it, inserted] = _shaders.try_emplace(shaderFile);
  if (!inserted) {
    return it->second;
  }

  const lib::Buffer<std::byte> shaderData =
      _fileLoader->loadFileToBuffer((std::filesystem::path(SHADERS_PATH) / shaderFile).string());
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

std::reference_wrapper<const PipelineLayout> PipelineManager::getOrCreatePipelineLayout(
    const PipelineLayoutKey& key, const LogicalDevice& logicalDevice) {
  auto [it, inserted] = _pipelineLayouts.try_emplace(key);

  if (!inserted) {
    return it->second;
  }

  PipelineLayout layout = PipelineLayout::create(
      logicalDevice, key.descriptorSetLayouts, key.pushConstants, key.createFlags);
  return it->second = std::move(layout);
}

namespace {

template <typename T>
constexpr VkPushConstantRange getPushConstantRange(
    VkShaderStageFlags shaderStages, uint32_t offset = 0) {
  return VkPushConstantRange{.stageFlags = shaderStages, .offset = offset, .size = sizeof(T)};
}

}  // namespace

GraphicsPipelineBuilder PipelineManager::createPBRProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex =
      addShader(logicalDevice, "shader_pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const PipelineLayout& pipelineLayout = getOrCreatePipelineLayout(
      PipelineLayoutKey{
        {getOrCreateBindlessLayout(logicalDevice), getOrCreateCameraLayout(logicalDevice)},
        {getPushConstantRange<PushConstantsPBR>(
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}
  }, logicalDevice);

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

  return GraphicsPipelineBuilder(std::move(
      GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
          .withRenderpass(renderpass)
          .withPipelineLayout(pipelineLayout)
          .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
          .withShaderStageCreateInfo(shaderStages)
          .withViewportStateCreateInfo()
          .withRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT)
          .withMultisampleStateCreateInfo(
              renderpass.getAttachmentsLayout().getNumMsaaSamples(), 0.2f)
          .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
          .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}

GraphicsPipelineBuilder PipelineManager::createPbrEnvMappingProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex =
      addShader(logicalDevice, "pbr_env_mapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shader_pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const PipelineLayout& pipelineLayout = getOrCreatePipelineLayout(
      PipelineLayoutKey{{getOrCreateBindlessLayout(logicalDevice)},
                        {getPushConstantRange<PushConstantsPBR>(
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}
      }, logicalDevice);

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

  return GraphicsPipelineBuilder(std::move(
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

GraphicsPipelineBuilder PipelineManager::createSkyboxProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex = addShader(logicalDevice, "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const PipelineLayout& pipelineLayout = getOrCreatePipelineLayout(
      PipelineLayoutKey{{getOrCreateBindlessLayout(logicalDevice)},
                        {getPushConstantRange<PushConstantsSkybox>(
                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)}
      }, logicalDevice);

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

  return GraphicsPipelineBuilder(std::move(
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

GraphicsPipelineBuilder PipelineManager::createShadowProgram(const Renderpass& renderpass) {
  const LogicalDevice& logicalDevice = renderpass.getLogicalDevice();
  const Shader& vertex = addShader(logicalDevice, "shadow.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
  const Shader& fragment =
      addShader(logicalDevice, "shadow.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

  const PipelineLayout& pipelineLayout = getOrCreatePipelineLayout(
      PipelineLayoutKey{
      {},
      {getPushConstantRange<PushConstantsShadow>(VK_SHADER_STAGE_VERTEX_BIT)}
      }, logicalDevice);

  lib::Buffer<VkPipelineColorBlendAttachmentState> colorBlendAttachments(
      renderpass.getAttachmentsLayout().getColorAttachmentsCount(),
      VkPipelineColorBlendAttachmentState{.colorWriteMask = VK_COLOR_COMPONENT_R_BIT});

  static constexpr VkVertexInputBindingDescription bindingDescriptions[] = {
    getBindingDescription<VertexP>()};
  static constexpr std::array attributeDescriptions = getAttributeDescriptions<VertexP>();

  const VkPipelineShaderStageCreateInfo shaderStages[] = {
    vertex.getVkPipelineStageCreateInfo(), fragment.getVkPipelineStageCreateInfo()};

  return GraphicsPipelineBuilder(std::move(
      GraphicsPipelineBuilder({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
          .withRenderpass(renderpass)
          .withPipelineLayout(pipelineLayout)
          .withInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
          .withVertexInputStateCreateInfo(bindingDescriptions, attributeDescriptions)
          .withShaderStageCreateInfo(shaderStages)
          .withViewportStateCreateInfo()
          .withRasterizationStateCreateInfo(
              VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, std::pair{0.7f, 2.0f})
          .withMultisampleStateCreateInfo(renderpass.getAttachmentsLayout().getNumMsaaSamples())
          .withColorBlendStateCreateInfo(std::move(colorBlendAttachments))
          .withDepthStencilStateCreateInfo(VK_COMPARE_OP_LESS_OR_EQUAL)));
}
