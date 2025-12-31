#include "graphics_pipeline_builder.h"

#include <algorithm>
#include <array>

#include "common/util/engine_exception.h"

// We need to pass the dynamic state in the constructor because some other states depend on it.
GraphicsPipelineBuilder::GraphicsPipelineBuilder(
    lib::Buffer<VkDynamicState>&& dynamicStates, VkPipelineDynamicStateCreateFlags flags)
  : _dynamicStates(std::move(dynamicStates)) {
  _dynamicState = VkPipelineDynamicStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .flags = flags,
    .dynamicStateCount = static_cast<uint32_t>(_dynamicStates.size()),
    .pDynamicStates = _dynamicStates.data()};
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(
    std::initializer_list<VkDynamicState> dynamicStates, VkPipelineDynamicStateCreateFlags flags)
  : GraphicsPipelineBuilder(lib::Buffer<VkDynamicState>(dynamicStates), flags) {}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(
    std::span<const VkDynamicState> dynamicStates, VkPipelineDynamicStateCreateFlags flags)
  : GraphicsPipelineBuilder(lib::Buffer<VkDynamicState>(dynamicStates), flags) {}

Pipeline GraphicsPipelineBuilder::createPipeline(
    const Renderpass& renderpass, const PipelineLayout& pipelineLayout) {
  if (&renderpass.getLogicalDevice() != &pipelineLayout.getLogicalDevice()) [[unlikely]] {
    throw EngineException(
        "Cannot create graphics pipeline with renderpass and pipeline layout from different "
        "logical devices.");
  }

  const VkGraphicsPipelineCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .stageCount = static_cast<uint32_t>(_shaderStages.size()),
    .pStages = _shaderStages.data(),
    .pVertexInputState = &_vertexInputState,
    .pInputAssemblyState = &_inputAssemblyState,
    .pTessellationState = &_tessellationState,
    .pViewportState = &_viewportState,
    .pRasterizationState = &_rasterizationState,
    .pMultisampleState = &_multisampleState,
    .pDepthStencilState = &_depthStencilState,
    .pColorBlendState = &_colorBlendState,
    .pDynamicState = &_dynamicState,
    .layout = pipelineLayout.getVkPipelineLayout(),
    .renderPass = renderpass.getVkRenderPass()};
  return Pipeline::create(renderpass.getLogicalDevice(), createInfo);
}

std::vector<Pipeline> GraphicsPipelineBuilder::createPipelines(
    std::span<const Renderpass> renderpasses, std::span<const GraphicsPipelineBuilder> builders,
    std::span<const PipelineLayout> pipelineLayouts) {
  if (builders.empty()) [[unlikely]] {
    return {};
  }

  if (builders.size() != pipelineLayouts.size()) [[unlikely]] {
    throw EngineException(
        "Cannot create graphics pipelines: number of builders does not match number of pipeline "
        "layouts.");
  }

  if (builders.size() != renderpasses.size()) [[unlikely]] {
    throw EngineException(
        "Cannot create graphics pipelines: number of builders does not match number of "
        "renderpasses.");
  }

  if (std::all_of(renderpasses.cbegin(), renderpasses.cend(),
                  [&firstDevice = renderpasses[0].getLogicalDevice()](const Renderpass& rp) {
                    return &rp.getLogicalDevice() == &firstDevice;
                  })
      == false) [[unlikely]] {
    throw EngineException(
        "Cannot create graphics pipelines: not all renderpasses belong to the same logical "
        "device.");
  }

  std::vector<VkGraphicsPipelineCreateInfo> createInfos;
  createInfos.reserve(builders.size());

  for (size_t i = 0; i < builders.size(); i++) {
    createInfos.push_back(VkGraphicsPipelineCreateInfo{
      .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .stageCount = static_cast<uint32_t>(builders[i]._shaderStages.size()),
      .pStages = builders[i]._shaderStages.data(),
      .pVertexInputState = &builders[i]._vertexInputState,
      .pInputAssemblyState = &builders[i]._inputAssemblyState,
      .pTessellationState = &builders[i]._tessellationState,
      .pViewportState = &builders[i]._viewportState,
      .pRasterizationState = &builders[i]._rasterizationState,
      .pMultisampleState = &builders[i]._multisampleState,
      .pDepthStencilState = &builders[i]._depthStencilState,
      .pColorBlendState = &builders[i]._colorBlendState,
      .pDynamicState = &builders[i]._dynamicState,
      .layout = pipelineLayouts[i].getVkPipelineLayout(),
      .renderPass = renderpasses[i].getVkRenderPass()});
  }

  return Pipeline::create(renderpasses[0].getLogicalDevice(), createInfos);
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withShaderStageCreateInfo(
    lib::Buffer<VkPipelineShaderStageCreateInfo>&& shaderStages,
    std::optional<SpecializationData> specializationData) {
  _shaderStages = std::move(shaderStages);
  if (specializationData.has_value()) {
    _specializationData = std::move(*specializationData);
  }

  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withShaderStageCreateInfo(
    std::span<const VkPipelineShaderStageCreateInfo> shaderStages,
    std::optional<SpecializationData> specializationData) {
  return withShaderStageCreateInfo(
      lib::Buffer<VkPipelineShaderStageCreateInfo>(shaderStages), specializationData);
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withVertexInputStateCreateInfo(
    lib::Buffer<VkVertexInputBindingDescription>&& vertexInputBindingDescriptions,
    lib::Buffer<VkVertexInputAttributeDescription>&& vertexInputAttributeDescriptions) {
  _vertexBindingDescriptions = std::move(vertexInputBindingDescriptions);
  _vertexAttributeDescriptions = std::move(vertexInputAttributeDescriptions);
  _vertexInputState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = static_cast<uint32_t>(_vertexBindingDescriptions.size()),
    .pVertexBindingDescriptions = _vertexBindingDescriptions.data(),
    .vertexAttributeDescriptionCount = static_cast<uint32_t>(_vertexAttributeDescriptions.size()),
    .pVertexAttributeDescriptions = _vertexAttributeDescriptions.data(),
  };

  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withVertexInputStateCreateInfo(
    std::span<const VkVertexInputBindingDescription> vertexInputBindingDescriptions,
    std::span<const VkVertexInputAttributeDescription> vertexInputAttributeDescriptions) {
  return withVertexInputStateCreateInfo(
      lib::Buffer<VkVertexInputBindingDescription>(vertexInputBindingDescriptions),
      lib::Buffer<VkVertexInputAttributeDescription>(vertexInputAttributeDescriptions));
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withInputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable,
    VkPipelineInputAssemblyStateCreateFlags flags) {
  _inputAssemblyState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .flags = flags,
    .topology = topology,
    .primitiveRestartEnable = primitiveRestartEnable};

  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withTessellationStateCreateInfo(
    uint32_t patchControlPoints, VkPipelineTessellationStateCreateFlags flags) {
  _tessellationState = {.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
                        .flags = flags,
                        .patchControlPoints = patchControlPoints};
  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withViewportStateCreateInfo(
    VkPipelineViewportStateCreateFlags flags, std::span<const VkViewport> viewports,
    std::span<const VkRect2D> scissors) {
  _viewports = lib::Buffer<VkViewport>(viewports);
  _scissors = lib::Buffer<VkRect2D>(scissors);
  // TODO: Check multiviewport feature and handle properly dynamic state.
  const bool dynamicScissor =
      std::find(_dynamicStates.cbegin(), _dynamicStates.cend(), VK_DYNAMIC_STATE_SCISSOR)
      != _dynamicStates.cend();
  const bool dynamicViewport =
      std::find(_dynamicStates.cbegin(), _dynamicStates.cend(), VK_DYNAMIC_STATE_VIEWPORT)
      != _dynamicStates.cend();

  _viewportState = VkPipelineViewportStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .flags = flags,
    .viewportCount = dynamicViewport ? 1 : static_cast<uint32_t>(_viewports.size()),
    .pViewports = dynamicViewport ? nullptr : _viewports.data(),
    .scissorCount = dynamicScissor ? 1 : static_cast<uint32_t>(_scissors.size()),
    .pScissors = dynamicScissor ? nullptr : _scissors.data()};
  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withRasterizationStateCreateInfo(
    VkPolygonMode polygonMode, VkCullModeFlags cullMode,
    std::optional<std::pair<float, float>> depthBiasConstantSlopeFactors, float depthBiasClamp,
    float lineWidth) {
  _rasterizationState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = depthBiasClamp != 0.0f ? VK_TRUE : VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = polygonMode,
    .cullMode = cullMode,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = depthBiasConstantSlopeFactors.has_value() ? VK_TRUE : VK_FALSE,
    .depthBiasConstantFactor =
        depthBiasConstantSlopeFactors.has_value() ? depthBiasConstantSlopeFactors->first : 0.0f,
    .depthBiasClamp = depthBiasClamp,
    .depthBiasSlopeFactor =
        depthBiasConstantSlopeFactors.has_value() ? depthBiasConstantSlopeFactors->second : 0.0f,
    .lineWidth = lineWidth};
  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withMultisampleStateCreateInfo(
    VkSampleCountFlagBits numMsaaSamples, std::optional<float> minSampleShading,
    std::span<const VkSampleMask> sampleMasks, VkPipelineMultisampleStateCreateFlags flags) {
  _sampleMasks = lib::Buffer<VkSampleMask>(sampleMasks);
  _multisampleState = VkPipelineMultisampleStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .flags = flags,
    .rasterizationSamples = numMsaaSamples,
    .sampleShadingEnable = minSampleShading.has_value(),
    .minSampleShading = minSampleShading.value_or(0.0f),
    .pSampleMask = _sampleMasks.size() > 0 ? _sampleMasks.data() : nullptr};
  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withColorBlendStateCreateInfo(
    lib::Buffer<VkPipelineColorBlendAttachmentState>&& colorBlendAttachments,
    std::array<float, 4> blendConstants, std::optional<VkLogicOp> logicOp,
    VkPipelineColorBlendStateCreateFlags flags) {
  _colorBlendAttachments = std::move(colorBlendAttachments);
  _colorBlendState = VkPipelineColorBlendStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .flags = flags,
    .logicOpEnable = logicOp.has_value() ? VK_TRUE : VK_FALSE,
    .logicOp = logicOp.value_or(VK_LOGIC_OP_COPY),
    .attachmentCount = static_cast<uint32_t>(_colorBlendAttachments.size()),
    .pAttachments = _colorBlendAttachments.data(),
    .blendConstants = {blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]},
  };
  return *this;
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withColorBlendStateCreateInfo(
    std::span<const VkPipelineColorBlendAttachmentState> colorBlendAttachments,
    std::array<float, 4> blendConstants, std::optional<VkLogicOp> logicOp,
    VkPipelineColorBlendStateCreateFlags flags) {
  return withColorBlendStateCreateInfo(
      lib::Buffer<VkPipelineColorBlendAttachmentState>(colorBlendAttachments), blendConstants,
      logicOp, flags);
}

GraphicsPipelineBuilder& GraphicsPipelineBuilder::withDepthStencilStateCreateInfo(
    std::optional<VkCompareOp> depthCompareOp, std::optional<std::pair<float, float>> depthBounds,
    std::optional<std::pair<VkStencilOpState, VkStencilOpState>> frontBack,
    VkPipelineDepthStencilStateCreateFlags flags) {
  _depthStencilState = VkPipelineDepthStencilStateCreateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .flags = flags,
    .depthTestEnable = depthCompareOp.has_value() ? VK_TRUE : VK_FALSE,
    .depthWriteEnable = depthCompareOp.has_value() ? VK_TRUE : VK_FALSE,
    .depthCompareOp = depthCompareOp.value_or(VK_COMPARE_OP_ALWAYS),
    .depthBoundsTestEnable = depthBounds.has_value() ? VK_TRUE : VK_FALSE,
    .stencilTestEnable = frontBack.has_value() ? VK_TRUE : VK_FALSE,
    .front = frontBack.has_value() ? frontBack->first : VkStencilOpState{},
    .back = frontBack.has_value() ? frontBack->second : VkStencilOpState{},
    .minDepthBounds = depthBounds.has_value() ? depthBounds->first : 0.0f,
    .maxDepthBounds = depthBounds.has_value() ? depthBounds->second : 1.0f};
  return *this;
}
