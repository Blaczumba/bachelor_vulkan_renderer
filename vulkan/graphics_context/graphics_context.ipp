#include "vulkan/graphics_context/graphics_context.h"

#include "common/entity_component_system/component/material.h"
#include "common/entity_component_system/component/mesh.h"
#include "common/entity_component_system/component/transform.h"
#include "common/model_loader/obj_loader/obj_loader.h"
#include "common/model_loader/tiny_gltf_loader/tiny_gltf_loader.h"

namespace vlkn {

namespace {

Texture createSkybox(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
    const AssetManager::ImageData& imageData, VkFormat format, float samplerAnisotropy) {
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_COLOR_BIT)
          .withExtent(imageData.width, imageData.height)
          .withFormat(format)
          .withMipLevels(imageData.mipLevels)
          .withUsage(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
          .withLayerCount(6)
          .withAdditionalCreateInfoFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
          .withLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
          .buildImage(logicalDevice, commandBuffer, imageData.stagingBuffer.getVkBuffer(),
                      imageData.copyRegions);
  texture.addCreateVkImageView(0, imageData.mipLevels, 0, 6);
  return texture;
}

Texture createCubemap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,

                      VkImageAspectFlags aspect, VkFormat format, VkImageUsageFlags additionalUsage,
                      float samplerAnisotropy) {
  Texture texture =
      TextureBuilder()
          .withAspect(aspect)
          .withExtent(1024 * 4, 1024 * 4)
          .withFormat(format)
          .withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | additionalUsage)
          .withLayerCount(6)
          .withAdditionalCreateInfoFlags(VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
          .buildAttachment(logicalDevice, commandBuffer);
  texture.addCreateVkImageView(0, 1, 0, 6);
  return texture;
}

Texture createShadowmap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                        uint32_t width, uint32_t height, VkFormat format) {
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_DEPTH_BIT)
          .withExtent(width, height)
          .withFormat(format)
          .withUsage(VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
          .buildImageSampler(logicalDevice, commandBuffer);
  texture.addCreateVkImageView(0, 1, 0, 1);
  return texture;
}

Texture createTexture2D(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
    const AssetManager::ImageData& imageData, VkFormat format, float samplerAnisotropy) {
  Texture texture =
      TextureBuilder()
          .withAspect(VK_IMAGE_ASPECT_COLOR_BIT)
          .withExtent(imageData.width, imageData.height)
          .withFormat(format)
          .withMipLevels(imageData.mipLevels)
          .withUsage(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
                     | VK_IMAGE_USAGE_SAMPLED_BIT)
          .buildMipmapImage(logicalDevice, commandBuffer, imageData.stagingBuffer.getVkBuffer(),
                            imageData.copyRegions);
  texture.addCreateVkImageView(0, imageData.mipLevels, 0, 1);
  return texture;
}

}  // namespace

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::setup() {
  std::string data = _fileLoader.loadFileToString(MODELS_PATH "cone.obj");
  VertexData cubeData = loadObj(*_assetManager, "cube.obj", data);
  const std::vector<VertexData> sceneData =
      LoadGltfFromFile(*_assetManager, MODELS_PATH "sponza/scene.gltf");
  cubeData.diffuseTexture = {
    _assetManager->loadImageAsync(TEXTURES_PATH "cubemap_yokohama_rgba.ktx"),
    TEXTURES_PATH "cubemap_yokohama_rgba.ktx"};
  loadCubemap(cubeData);
  createDescriptorSets();
  createEnvMappingResources();
  createShadowResources();
  createGraphicsPipelines();
  createCommandBuffers();
  createSyncObjects();
  loadObjects(sceneData);
  createOctreeScene();
  {
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
    recordShadowCommandBuffer(handle.getCommandBuffer());
    recordEnvMappingCommandBuffer(handle.getCommandBuffer());
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::loadCubemap(
    const VertexData& cubeData) {
  SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
  const VkCommandBuffer commandBuffer = handle.getCommandBuffer();

  const AssetManager::ImageData& imageData =
      _assetManager->getImageData(cubeData.diffuseTexture.ID);

  _textureCubemap =
      createSkybox(*_logicalDevice, commandBuffer, imageData, VK_FORMAT_R8G8B8A8_UNORM,
                   _physicalDevice->getMaxSamplerAnisotropy());

  if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    AssetManager::VertexData vData = _assetManager->releaseVertexData(cubeData.vertexResourceID);
    _vertexBufferCubeHandle = _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("P")));
    _vertexBufferCubeNormalsHandle =
        _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("PN")));
    _indexBufferCubeHandle = _gpuBufferManager->transferBuffer(std::move(vData.indexBuffer));
    _indexBufferCubeType = vData.indexType;
  } else if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    const AssetManager::VertexData& vData = _assetManager->getVertexData(cubeData.vertexResourceID);
    _vertexBufferCubeHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.buffers.at("P"), GpuBufferManager::BufferType::VERTEX);
    _vertexBufferCubeNormalsHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.buffers.at("PN"), GpuBufferManager::BufferType::VERTEX);
    _indexBufferCubeHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.indexBuffer, GpuBufferManager::BufferType::INDEX);
    _indexBufferCubeType = vData.indexType;
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createDescriptorSets() {
  // If VR presentation is enabled then multiply times 2 otherwise times 1.
  const uint32_t size =
      _logicalDevice->getPhysicalDevice().getMemoryAlignment(sizeof(UniformBufferCamera) * (1 + MULTIVIEW_PRESENTATION));
  _dynamicUniformBuffersCamera =
      Buffer::createUniformBuffer(*_logicalDevice, MAX_FRAMES_IN_FLIGHT * size);
  Sampler sampler = SamplerBuilder()
                        .withAnisotropy(_physicalDevice->getMaxSamplerAnisotropy())
                        .build(*_logicalDevice);
  _skyboxHandle = _bindlessWriter->storeTexture(_textureCubemap, sampler);
  _samplerManager->transferSampler(std::move(sampler));

  _dynamicDescriptorSetWriter.storeDynamicBuffer(_dynamicUniformBuffersCamera, size);
  _dynamicDescriptorSetWriter.writeDescriptorSet(
      _logicalDevice->getVkDevice(), _dynamicDescriptorSet.getVkDescriptorSet());

  _lightBuffer = Buffer::createUniformBuffer(*_logicalDevice, sizeof(UniformBufferLight));
  _lightHandle = _bindlessWriter->storeBuffer(_lightBuffer);

  _ubLight.pos = glm::vec3(15.1891f, 2.66408f, -0.841221f);
  _ubLight.projView = glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 40.0f);
  _ubLight.projView[1][1] = -_ubLight.projView[1][1];
  _ubLight.projView = _ubLight.projView
                      * glm::lookAt(_ubLight.pos, glm::vec3(-3.82383f, 3.66503f, 1.30751f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
  _lightBuffer.copyData(_ubLight, 0);
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createEnvMappingResources() {
  // First pass for rendering the environment map.
  const float samplerAnisotropy = _physicalDevice->getMaxSamplerAnisotropy();
  {
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);

    _envMappingAttachments[0] = createCubemap(
        *_logicalDevice, handle.getCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, samplerAnisotropy);
    _envMappingAttachments[1] = createCubemap(
        *_logicalDevice, handle.getCommandBuffer(), VK_IMAGE_ASPECT_DEPTH_BIT, VK_FORMAT_D16_UNORM,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, samplerAnisotropy);
  }

  AttachmentLayout attachmentLayout;
  attachmentLayout.addColorAttachment(
      VK_FORMAT_R8G8B8A8_SRGB, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_STORE);
  attachmentLayout.addDepthAttachment(VK_FORMAT_D16_UNORM, VK_ATTACHMENT_STORE_OP_DONT_CARE);

  _envMappingRenderPass =
      RenderpassBuilder(attachmentLayout)
          .withMultiView({0b111111}, {0b111111})
          .addDependency(
              VK_SUBPASS_EXTERNAL, 0,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
          .addSubpass({0, 1})
          .build(*_logicalDevice);

  _envMappingFramebuffer =
      Framebuffer::createFromTextures(_envMappingRenderPass, _envMappingAttachments);

  const glm::vec3 pos = glm::vec3(0.0f, 2.0f, 0.0f);
  glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 50.0f);

  struct {
    alignas(16) glm::mat4 projView[6];
    alignas(16) glm::vec3 viewPos;
    alignas(16) glm::mat4 lightProjView;
    alignas(16) glm::vec3 lightPos;
  } const faceTransform = {
    .projView =
        {
                   proj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                   proj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                   proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                   proj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                   proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                   proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                   },
    .viewPos = pos,
    .lightProjView = _ubLight.projView,
    .lightPos = _ubLight.pos
  };

  _envMappingUniformBuffer = Buffer::createUniformBuffer(*_logicalDevice, sizeof(faceTransform));
  _envMappingUniformBuffer.copyData(faceTransform);
  _envMappingHandle = _bindlessWriter->storeBuffer(_envMappingUniformBuffer);
  Sampler sampler = SamplerBuilder().withAnisotropy(samplerAnisotropy).build(*_logicalDevice);
  _envMappingTextureHandle = _bindlessWriter->storeTexture(_envMappingAttachments[0], sampler);
  _samplerManager->transferSampler(std::move(sampler));
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createShadowResources() {
  {
    // TODO: Should not be in this function.
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
    const VkCommandBuffer commandBuffer = handle.getCommandBuffer();
    _shadowMap =
        createShadowmap(*_logicalDevice, commandBuffer, 1024 * 2, 1024 * 2, VK_FORMAT_D32_SFLOAT);
  }
  Sampler sampler =
      SamplerBuilder()
          .withCompareOp(VK_COMPARE_OP_LESS_OR_EQUAL)
          .withAddressMode(
              VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
              VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
          .withBorderColor(VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE)
          .build(*_logicalDevice);
  _shadowHandle = _bindlessWriter->storeTexture(_shadowMap, std::move(sampler));
  _samplerManager->transferSampler(std::move(sampler));

  AttachmentLayout attachmentLayout;
  attachmentLayout.addShadowAttachment(
      VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  _shadowRenderPass = RenderpassBuilder(attachmentLayout).addSubpass({0}).build(*_logicalDevice);
  _shadowFramebuffer =
      Framebuffer::createFromTextures(_shadowRenderPass, std::span(&_shadowMap, 1));
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createGraphicsPipelines() {
  _graphicsPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createPBRProgram(_renderPass, MULTIVIEW_PRESENTATION));
  _skyboxPipeline =
      _pipelineManager->getPipeline(_pipelineManager->createSkyboxProgram(_renderPass));
  _phongEnvMappingPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createEnvMappingProgram(_renderPass, MULTIVIEW_PRESENTATION));
  _shadowPipeline =
      _pipelineManager->getPipeline(_pipelineManager->createShadowProgram(_shadowRenderPass));
  _envMappingPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createPbrEnvMappingProgram(_envMappingRenderPass));
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createCommandBuffers() {
  for (int i = 0; i <= MAX_THREADS_IN_POOL; i++) {
    _commandPools[i] =
        CommandPool::create(*_logicalDevice, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  }
  _primaryCommandBuffer =
      _commandPools[MAX_THREADS_IN_POOL]->createCommandBuffers<MAX_FRAMES_IN_FLIGHT>(
          VK_COMMAND_BUFFER_LEVEL_PRIMARY);
  for (int i = 0; i < MAX_THREADS_IN_POOL; i++) {
    _secondaryCommandBuffers[i] = _commandPools[i]->createCommandBuffers<MAX_FRAMES_IN_FLIGHT>(
        VK_COMMAND_BUFFER_LEVEL_SECONDARY);
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createSyncObjects() {
  static constexpr VkSemaphoreCreateInfo semaphoreInfo = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  static constexpr VkFenceCreateInfo fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  if constexpr (!SYNCED_OUTSIDE) {
    // TODO: Swapchain images count.
    _synchContext.renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  }
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if constexpr (!SYNCED_OUTSIDE) {
      CHECK_VKCMD(vkCreateSemaphore(_logicalDevice->getVkDevice(), &semaphoreInfo, nullptr,
                                    &_synchContext.imageAvailableSemaphores[i]),
                  "Failed to create VkSemaphore.");
      CHECK_VKCMD(vkCreateSemaphore(_logicalDevice->getVkDevice(), &semaphoreInfo, nullptr,
                                    &_synchContext.renderFinishedSemaphores[i]),
                  "Failed to create VkSemaphore.");
    }
    CHECK_VKCMD(vkCreateFence(_logicalDevice->getVkDevice(), &fenceInfo, nullptr, &_frameFences[i]),
                "Failed to create VkFence.");
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
std::tuple<UniformTextureHandle, GpuTextureHandle>
GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::getOrLoadTexture(
    std::unordered_map<StagingImageDataResourceHandle,
                       std::pair<UniformTextureHandle, GpuTextureHandle>>& textureCache,
    StagingImageDataResourceHandle textureID, VkFormat format, VkCommandBuffer commandBuffer,
    float maxSamplerAnisotropy, SamplerHandle samplerHandle) {
  auto it = textureCache.find(textureID);

  if (it != textureCache.end()) {
    _gpuBufferManager->increaseRefCount(it->second.second);
    return it->second;
  }

  const AssetManager::ImageData& imgData = _assetManager->getImageData(textureID);
  Texture texture =
      createTexture2D(*_logicalDevice, commandBuffer, imgData, format, maxSamplerAnisotropy);
  UniformTextureHandle handle =
      _bindlessWriter->storeTexture(texture, _samplerManager->getSampler(samplerHandle));
  const GpuTextureHandle index = _gpuBufferManager->transferTexture(std::move(texture));

  const auto result = std::make_tuple(handle, index);
  textureCache.emplace(textureID, result);

  return result;
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::loadObjects(
    std::span<const VertexData> sceneData) {
  const float maxSamplerAnisotropy = _physicalDevice->getMaxSamplerAnisotropy();
  _objects.reserve(sceneData.size());

  std::unordered_map<StagingImageDataResourceHandle,
                     std::pair<UniformTextureHandle, GpuTextureHandle>>
      textureCache;
  textureCache.reserve(sceneData.size());
  SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
  const VkCommandBuffer commandBuffer = handle.getCommandBuffer();
  Sampler sampler = SamplerBuilder()
                        .withAnisotropy(_physicalDevice->getMaxSamplerAnisotropy())
                        .withLodRange(0.0f, VK_LOD_CLAMP_NONE)
                        .build(*_logicalDevice);
  SamplerHandle samplerHandle = _samplerManager->transferSampler(std::move(sampler));

  for (const VertexData& sceneObject : sceneData) {
    const auto [diffuseHandle, diffuseTextureIndex] =
        getOrLoadTexture(textureCache, sceneObject.diffuseTexture.ID, VK_FORMAT_R8G8B8A8_SRGB,
                         commandBuffer, maxSamplerAnisotropy, samplerHandle);

    const auto [normalHandle, normalTextureIndex] =
        getOrLoadTexture(textureCache, sceneObject.normalTexture.ID, VK_FORMAT_R8G8B8A8_UNORM,
                         commandBuffer, maxSamplerAnisotropy, samplerHandle);

    const auto [metallicRoughnessHandle, metallicRoughnessTextureIndex] = getOrLoadTexture(
        textureCache, sceneObject.metallicRoughnessTexture.ID, VK_FORMAT_R8G8B8A8_UNORM,
        commandBuffer, maxSamplerAnisotropy, samplerHandle);

    Entity e = _registry.createEntity();
    _objects.emplace_back("", e);
    _registry.addComponent<MaterialComponent>(
        e, MaterialComponent{diffuseHandle, normalHandle, metallicRoughnessHandle});
    MeshComponent msh;
    if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
      AssetManager::VertexData vData =
          _assetManager->releaseVertexData(sceneObject.vertexResourceID);
      msh.vertexBufferHandle =
          _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("PTNT")));
      msh.vertexBufferPrimitiveHandle =
          _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("P")));
      msh.indexBufferHandle = _gpuBufferManager->transferBuffer(std::move(vData.indexBuffer));
      msh.indexType = vData.indexType;
    } else if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      const AssetManager::VertexData& vData =
          _assetManager->getVertexData(sceneObject.vertexResourceID);
      msh.vertexBufferHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.buffers.at("PTNT"), GpuBufferManager::BufferType::VERTEX);
      msh.vertexBufferPrimitiveHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.buffers.at("P"), GpuBufferManager::BufferType::VERTEX);
      msh.indexBufferHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.indexBuffer, GpuBufferManager::BufferType::INDEX);
      msh.indexType = vData.indexType;
    }
    msh.aabb = createAABBfromVertices(sceneObject.positions, sceneObject.model);
    _registry.addComponent<MeshComponent>(e, std::move(msh));

    TransformComponent trsf;
    trsf.model = sceneObject.model;
    _registry.addComponent<TransformComponent>(e, std::move(trsf));
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createOctreeScene() {
  AABB sceneAABB = _registry.getComponent<MeshComponent>(_objects[0].getEntity()).aabb;

  for (int i = 1; i < _objects.size(); ++i) {
    sceneAABB.extend(_registry.getComponent<MeshComponent>(_objects[i].getEntity()).aabb);
  }
  _octree = std::make_unique<Octree>(sceneAABB);

  for (const Object& object : _objects) {
    _octree->addObject(&object, _registry.getComponent<MeshComponent>(object.getEntity()).aabb);
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::recordShadowCommandBuffer(
    VkCommandBuffer commandBuffer) {
  const VkCommandBufferBeginInfo beginInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  VkExtent2D extent = _shadowMap.getVkExtent2D();

  std::span<const VkClearValue> clearValues =
      _shadowRenderPass.getAttachmentsLayout().getVkClearValues();

  const VkRenderPassBeginInfo renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = _shadowRenderPass.getVkRenderPass(),
    .framebuffer = _shadowFramebuffer.getVkFramebuffer(),
    .renderArea = {.offset = {0, 0}, .extent = extent},
    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
    .pClearValues = clearValues.data()
  };

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  const VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(extent.width),
    .height = static_cast<float>(extent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  const VkRect2D scissor = {
    .offset = {0, 0},
      .extent = extent
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  const VkDeviceSize offsets[] = {0};
  vkCmdBindPipeline(
      commandBuffer, _shadowPipeline->getVkPipelineBindPoint(), _shadowPipeline->getVkPipeline());

  PushConstantsShadow pc = {.lightProjView = _ubLight.projView};

  for (const Object& object : _objects) {
    const auto& meshComponent = _registry.getComponent<MeshComponent>(object.getEntity());
    const auto& transformComponent = _registry.getComponent<TransformComponent>(object.getEntity());

    pc.model = transformComponent.model;

    vkCmdPushConstants(commandBuffer, _shadowPipeline->getVkPipelineLayout(),
                       VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

    VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(meshComponent.vertexBufferPrimitiveHandle).getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    const Buffer& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, meshComponent.indexType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.getSize() / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(commandBuffer);
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::recordEnvMappingCommandBuffer(
    VkCommandBuffer commandBuffer) {
  const VkCommandBufferBeginInfo beginInfo = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  VkExtent2D extent = _envMappingAttachments[0].getVkExtent2D();

  std::span<const VkClearValue> clearValues =
      _envMappingRenderPass.getAttachmentsLayout().getVkClearValues();

  const VkRenderPassBeginInfo renderPassInfo = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .renderPass = _envMappingRenderPass.getVkRenderPass(),
    .framebuffer = _envMappingFramebuffer.getVkFramebuffer(),
    .renderArea = {.offset = {0, 0}, .extent = extent},
    .clearValueCount = static_cast<uint32_t>(clearValues.size()),
    .pClearValues = clearValues.data()
  };

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  const VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = static_cast<float>(extent.width),
    .height = static_cast<float>(extent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

  const VkRect2D scissor = {
    .offset = {0, 0},
      .extent = extent
  };
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  const VkDescriptorSet descriptorSets[] = {_bindlessDescriptorSet.getVkDescriptorSet()};

  vkCmdBindPipeline(commandBuffer, _envMappingPipeline->getVkPipelineBindPoint(),
                    _envMappingPipeline->getVkPipeline());

  vkCmdBindDescriptorSets(
      commandBuffer, _envMappingPipeline->getVkPipelineBindPoint(),
      _envMappingPipeline->getVkPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

  const VkDeviceSize offsets[] = {0};

  for (const Object& object : _objects) {
    const auto& meshComponent = _registry.getComponent<MeshComponent>(object.getEntity());
    const auto& transformComponent = _registry.getComponent<TransformComponent>(object.getEntity());
    const auto& materialComponent = _registry.getComponent<MaterialComponent>(object.getEntity());

    const PushConstantsModelDescriptorHandles32Bit pc = {
      .model = transformComponent.model,
      .descriptorHandles = {static_cast<uint32_t>(*_envMappingHandle),
                            static_cast<uint32_t>(*materialComponent.diffuse),
                            static_cast<uint32_t>(*materialComponent.normal),
                            static_cast<uint32_t>(*materialComponent.metallicRoughness),
                            static_cast<uint32_t>(*_shadowHandle)}
    };

    vkCmdPushConstants(
        commandBuffer, _envMappingPipeline->getVkPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

    VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle).getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    const Buffer& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, meshComponent.indexType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.getSize() / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(commandBuffer);
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::updateUniformBuffer(
    const std::vector<common::CameraContext>& cameraContexts, uint32_t currentFrame) {
  if constexpr (MULTIVIEW_PRESENTATION) {
    UniformBufferCamera _ubCamera[2];
    for (size_t i = 0; i < cameraContexts.size(); ++i) {
      _ubCamera[i].view = cameraContexts[i].view;
      _ubCamera[i].proj = cameraContexts[i].proj;
      _ubCamera[i].pos = cameraContexts[i].position;
      _dynamicUniformBuffersCamera.copyData(
          _ubCamera,
          currentFrame * _physicalDevice->getMemoryAlignment(2 * sizeof(UniformBufferCamera)));
    }

  } else {
    UniformBufferCamera _ubCamera;
    _ubCamera.view = cameraContexts[0].view;
    _ubCamera.proj = cameraContexts[0].proj;
    _ubCamera.pos = cameraContexts[0].position;
    _dynamicUniformBuffersCamera.copyData(
        _ubCamera,
        currentFrame * _physicalDevice->getMemoryAlignment(sizeof(UniformBufferCamera)));
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::
    recordOctreeSecondaryCommandBuffer(
    const VkCommandBuffer commandBuffer, const OctreeNode* rootNode,
    std::span<const glm::vec4> planes) {
  if (!rootNode || !rootNode->getVolume().intersectsFrustum(planes)) {
    return;
  }

  static std::queue<const OctreeNode*> nodeQueue;  // Keep it static to preserve
  // capacity
  nodeQueue.push(rootNode);

  while (!nodeQueue.empty()) {
    const OctreeNode* node = nodeQueue.front();
    nodeQueue.pop();

    for (const Object* object : node->getObjects()) {
      const auto& materialComponent =
          _registry.getComponent<MaterialComponent>(object->getEntity());
      const auto& transformComponent =
          _registry.getComponent<TransformComponent>(object->getEntity());

      const PushConstantsModelDescriptorHandles32Bit pc = {
        .model = transformComponent.model,
        .descriptorHandles = {
                              static_cast<uint32_t>(*_lightHandle), static_cast<uint32_t>(*materialComponent.diffuse),
                              static_cast<uint32_t>(*materialComponent.normal),
                              static_cast<uint32_t>(*materialComponent.metallicRoughness),
                              static_cast<uint32_t>(*_shadowHandle)}
      };

      vkCmdPushConstants(
          commandBuffer, _graphicsPipeline->getVkPipelineLayout(),
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

      const auto& meshComponent = _registry.getComponent<MeshComponent>(object->getEntity());
      const Buffer& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
      const Buffer& vertexBuffer = _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle);
      static constexpr VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.getVkBuffer(), offsets);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, meshComponent.indexType);
      vkCmdDrawIndexed(
          commandBuffer, indexBuffer.getSize() / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
    }

    static constexpr OctreeNode::Subvolume options[] = {
      OctreeNode::Subvolume::LOWER_LEFT_BACK,  OctreeNode::Subvolume::LOWER_LEFT_FRONT,
      OctreeNode::Subvolume::LOWER_RIGHT_BACK, OctreeNode::Subvolume::LOWER_RIGHT_FRONT,
      OctreeNode::Subvolume::UPPER_LEFT_BACK,  OctreeNode::Subvolume::UPPER_LEFT_FRONT,
      OctreeNode::Subvolume::UPPER_RIGHT_BACK, OctreeNode::Subvolume::UPPER_RIGHT_FRONT};

    for (OctreeNode::Subvolume option : options) {
      const OctreeNode* childNode = node->getChild(option);
      if (childNode && childNode->getVolume().intersectsFrustum(planes)) {
        nodeQueue.push(childNode);
      }
    }
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::recordCommandBuffer(
    const glm::mat4& cameraProj, const glm::mat4& cameraView, uint32_t imageIndex) {
  const Framebuffer& framebuffer = _framebuffers[imageIndex];
  const CommandBuffer& primaryCommandBuffer = _primaryCommandBuffer[_synchContext.currentFrame];
  primaryCommandBuffer.beginAsPrimary();
  primaryCommandBuffer.beginRenderPass(framebuffer);

  static const bool viewportScissorInheritance =
      _physicalDevice->hasAvailableExtension(VK_NV_INHERITED_VIEWPORT_SCISSOR_EXTENSION_NAME);

  VkCommandBufferInheritanceViewportScissorInfoNV scissorViewportInheritance;
  if (viewportScissorInheritance) [[likely]] {
    scissorViewportInheritance = VkCommandBufferInheritanceViewportScissorInfoNV{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV,
      .viewportScissor2D = VK_TRUE,
      .viewportDepthCount = 1,
      .pViewportDepths = &framebuffer.getViewport(),
    };
  }

  std::future<void> futures[MAX_THREADS_IN_POOL];

  futures[0] = std::async(std::launch::async, [&]() -> void {
    const VkCommandBuffer commandBuffer =
        _secondaryCommandBuffers[0][_synchContext.currentFrame].getVkCommandBuffer();

    if (viewportScissorInheritance) [[likely]] {
      _secondaryCommandBuffers[0][_synchContext.currentFrame].beginAsSecondary(
          framebuffer, &scissorViewportInheritance);
    } else {
      _secondaryCommandBuffers[0][_synchContext.currentFrame].beginAsSecondary(
          framebuffer, nullptr);
      vkCmdSetViewport(commandBuffer, 0, 1, &framebuffer.getViewport());
      vkCmdSetScissor(commandBuffer, 0, 1, &framebuffer.getScissor());
    }
    vkCmdBindPipeline(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(),
                      _graphicsPipeline->getVkPipeline());

    const OctreeNode* root = _octree->getRoot();
    const auto& planes = extractFrustumPlanes(cameraProj * cameraView);

    VkDescriptorSet descriptorSets[] = {
      _bindlessDescriptorSet.getVkDescriptorSet(), _dynamicDescriptorSet.getVkDescriptorSet()};

    uint32_t offset;

    _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
        &offset, {_synchContext.currentFrame});

    vkCmdBindDescriptorSets(
        commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(),
        _graphicsPipeline->getVkPipelineLayout(), 0,
        static_cast<uint32_t>(std::size(descriptorSets)), descriptorSets, 1, &offset);

    recordOctreeSecondaryCommandBuffer(commandBuffer, root, planes);

    CHECK_VKCMD(vkEndCommandBuffer(commandBuffer), "Failed to vkEndCommandBuffer.");
  });

  futures[1] = std::async(std::launch::async, [&]() -> void {
    // Skybox
    const VkCommandBuffer commandBuffer =
        _secondaryCommandBuffers[1][_synchContext.currentFrame].getVkCommandBuffer();

    if (viewportScissorInheritance) [[likely]] {
      _secondaryCommandBuffers[1][_synchContext.currentFrame].beginAsSecondary(
          framebuffer, &scissorViewportInheritance);
    } else {
      _secondaryCommandBuffers[1][_synchContext.currentFrame].beginAsSecondary(
          framebuffer, nullptr);
      vkCmdSetViewport(commandBuffer, 0, 1, &framebuffer.getViewport());
      vkCmdSetScissor(commandBuffer, 0, 1, &framebuffer.getScissor());
    }

    vkCmdBindPipeline(
        commandBuffer, _skyboxPipeline->getVkPipelineBindPoint(), _skyboxPipeline->getVkPipeline());

    static constexpr VkDeviceSize offsets[] = {0};

    const VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(_vertexBufferCubeHandle).getVkBuffer();
    const Buffer& indexBuffer = _gpuBufferManager->getBuffer(_indexBufferCubeHandle);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, _indexBufferCubeType);

    const PushConstantsSkybox pc = {.proj = cameraProj,
                                    .view = cameraView,
                                    .skyboxHandle = static_cast<uint32_t>(*_skyboxHandle)};
    vkCmdPushConstants(
        commandBuffer, _skyboxPipeline->getVkPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pc), &pc);

    const VkDescriptorSet descriptorSets[] = {
      _bindlessDescriptorSet.getVkDescriptorSet(), _dynamicDescriptorSet.getVkDescriptorSet()};

    vkCmdBindDescriptorSets(
        commandBuffer, _skyboxPipeline->getVkPipelineBindPoint(),
        _skyboxPipeline->getVkPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.getSize() / getIndexSize(_indexBufferCubeType), 1, 0, 0, 0);

    // Env mapping
    vkCmdBindPipeline(commandBuffer, _phongEnvMappingPipeline->getVkPipelineBindPoint(),
                      _phongEnvMappingPipeline->getVkPipeline());

    uint32_t offset;

    _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
        &offset, {_synchContext.currentFrame});

    vkCmdBindDescriptorSets(commandBuffer, _phongEnvMappingPipeline->getVkPipelineBindPoint(),
                            _phongEnvMappingPipeline->getVkPipelineLayout(), 0,
                            std::size(descriptorSets), descriptorSets, 1, &offset);

    const PushConstantsModelDescriptorHandles32Bit envMapPc = {
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))
               * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)),
      .descriptorHandles = {
                            static_cast<uint32_t>(*_envMappingHandle), static_cast<uint32_t>(*_lightHandle)}
    };

    vkCmdPushConstants(
        commandBuffer, _phongEnvMappingPipeline->getVkPipelineLayout(),
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(envMapPc), &envMapPc);

    const VkBuffer vertexBufferCubeNormals =
        _gpuBufferManager->getBuffer(_vertexBufferCubeNormalsHandle).getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferCubeNormals, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, _indexBufferCubeType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.getSize() / getIndexSize(_indexBufferCubeType), 1, 0, 0, 0);

    CHECK_VKCMD(vkEndCommandBuffer(commandBuffer), "Failed to vkEndCommandBuffer.");
  });

  std::for_each(std::begin(futures), std::end(futures), [](std::future<void>& future) {
    future.wait();
  });

  primaryCommandBuffer.executeSecondaryCommandBuffers(
      {_secondaryCommandBuffers[0][_synchContext.currentFrame].getVkCommandBuffer(),
       _secondaryCommandBuffers[1][_synchContext.currentFrame].getVkCommandBuffer()});
  primaryCommandBuffer.endRenderPass();

  if (primaryCommandBuffer.end() != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

} // namespace vlkn
