#include "vulkan/graphics_context/graphics_context.h"

#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <span>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "common/abstractions/contexts.h"
#include "common/abstractions/graphics_context.h"
#include "common/entity_component_system/component/material.h"
#include "common/entity_component_system/component/mesh.h"
#include "common/entity_component_system/component/transform.h"
#include "common/entity_component_system/registry/registry.h"
#include "common/model_loader/model_loader.h"
#include "common/model_loader/obj_loader/obj_loader.h"
#include "common/model_loader/tiny_gltf_loader/tiny_gltf_loader.h"
#include "common/object/object.h"
#include "common/scene/octree.h"
#include "common/util/engine_exception.h"
#include "common/util/primitives.h"
#include "lib/bitwise.h"
#include "presentation_graphics_communication/presentation_graphics_communication.h"
#include "vulkan/graphics_context/presentation_lib.h"
#include "vulkan/resource_manager/asset_manager.h"
#include "vulkan/resource_manager/bindless_descriptor_set_writer.h"
#include "vulkan/resource_manager/framebuffer_attachments_manager.h"
#include "vulkan/resource_manager/gpu_buffer_manager.h"
#include "vulkan/resource_manager/pipeline_manager.h"
#include "vulkan/resource_manager/sampler_manager.h"
#include "vulkan/wrapper/command_buffer/command_buffer.h"
#include "vulkan/wrapper/debug_messenger/debug_messenger.h"
#include "vulkan/wrapper/descriptor_set/descriptor_pool.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_writer.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/instance/instance.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/physical_device/physical_device.h"
#include "vulkan/wrapper/render_pass/render_pass.h"
#include "vulkan/wrapper/util/index_buffer_util.h"

#define GCONTEXT_TEMPLATE template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
#define GCONTEXT_CLASS    GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::

namespace vlkn {

namespace {

Framebuffer createFramebufferFromTextures(
    const Renderpass& renderpass, std::span<const Texture> textures) {
  std::vector<VkImageView> imageViews;
  imageViews.reserve(textures.size());
  std::optional<VkExtent2D> extent;
  for (const Texture& texture : textures) {
    imageViews.push_back(texture.getVkImageView());
    if (!extent.has_value()) {
      extent = texture.getVkExtent2D();
    } else if (VkExtent2D tmpExtent = texture.getVkExtent2D();
               extent->width != tmpExtent.width || extent->height != tmpExtent.height) {
      throw EngineException("All images must have the same size to create a Framebuffer.");
    }
  }

  if (!extent.has_value()) {
    throw EngineException("Framebuffer must have an attachment.");
  }

  return Framebuffer::create(renderpass, *extent, imageViews);
}

Texture createSkybox(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
    const AssetManager::ImageData& imageData, VkFormat format, float samplerAnisotropy);

Texture createCubemap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,

                      VkImageAspectFlags aspect, VkFormat format, VkImageUsageFlags additionalUsage,
                      float samplerAnisotropy);

Texture createShadowmap(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                        uint32_t width, uint32_t height, VkFormat format);

Texture createTexture2D(
    const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
    const AssetManager::ImageData& imageData, VkFormat format, float samplerAnisotropy);

Texture createAttachment(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                         VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent,
                         uint32_t numLayers, VkImageAspectFlags aspect, VkImageUsageFlags usage);

void createFsrContents(
    Texture& texture, const LogicalDevice& logicalDevice, const CommandPool& commandPool);

}  // namespace

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS setup() {
  const std::vector<common::VertexData> sponzaData =
      common::LoadGltfFromFile(*_assetManager, MODELS_PATH "sponza/scene.gltf");
  //     std::vector<common::VertexData> antiqueCandleStickData = common::LoadGltfFromFile(
  //         *_assetManager, MODELS_PATH "ornate_antique_candlestick/scene.gltf");
  //     std::for_each(
  //         antiqueCandleStickData.begin(), antiqueCandleStickData.end(), [](common::VertexData&
  //         data) {
  //           data.model = data.model * glm::translate(glm::mat4(1.0f), glm::vec3(7.0f, -2.0f,
  //           .0f))
  //                        * glm::scale(glm::mat4(1.0f), glm::vec3(0.02f, 0.02f, 0.02f));
  //         });
  /*std::vector<common::VertexData> lanternData =
      common::LoadGltfFromFile(*_assetManager, MODELS_PATH "ornate_lantern_3d_model/scene.gltf");
  std::for_each(lanternData.begin(), lanternData.end(), [](common::VertexData& data) {
    data.model = data.model * glm::translate(glm::mat4(1.0f), glm::vec3(8.5f, 4.25f, 0.0f))
                 * glm::scale(glm::mat4(1.0f), glm::vec3(1.5f, 1.5f, 1.5f));
  });*/
  // std::vector<common::VertexData> spartanData =
  //     common::LoadGltfFromFile(*_assetManager, MODELS_PATH "pbr_spartan_helmet/scene.gltf");
  // std::for_each(spartanData.begin(), spartanData.end(), [](common::VertexData& data) {
  //   data.model = data.model * glm::translate(glm::mat4(1.0f), glm::vec3(1000.0f, 16.0f,
  //   -250.0f))
  //                * glm::rotate(glm::mat4(1.0f), glm::radians(-25.0f), glm::vec3(1.0f, 0.0f,
  //                0.0f))
  //                * glm::scale(glm::mat4(1.0f), glm::vec3(2.5f, 2.5f, 2.5f));
  // });

  createDescriptorSets();
  createEnvMappingResources();
  createShadowResources();
  createGraphicsPipelines();
  createCommandBuffers();
  createSyncObjects();
  loadObjects(sponzaData, _graphicsPipelineHandle);
  // loadObjects(antiqueCandleStickData, _graphicsPipelineHandle);
  // loadObjects(lanternData, _graphicsTesselationPipelineHandle);
  // loadObjects(spartanData, _graphicsTesselationPipelineHandle);

  {
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
    const VkCommandBuffer commandBuffer = handle.getCommandBuffer();

    std::string cubeFileContents = _fileLoader.loadFileToString(MODELS_PATH "cube.obj");
    common::VertexData cubeData = common::loadObj(*_assetManager, "cube.obj", cubeFileContents);
    cubeData.diffuseTexture = {
      _assetManager->loadImageAsync(TEXTURES_PATH "cubemap_yokohama_rgba.ktx"),
      TEXTURES_PATH "cubemap_yokohama_rgba.ktx"};
    const AssetManager::ImageData& imageData =
        _assetManager->getImageData(cubeData.diffuseTexture.ID);

    Texture skyboxTexture =
        createSkybox(*_logicalDevice, commandBuffer, imageData, VK_FORMAT_R8G8B8A8_SRGB,
                     _physicalDevice->getMaxSamplerAnisotropy());
    _skyboxEntity =
        loadObject(commandBuffer, cubeData, PipelineHandle(0), std::move(skyboxTexture));

    // std::string razielFileContents = _fileLoader.loadFileToString(MODELS_PATH "Raziel.obj");
    // common::VertexData razielData =
    //     common::loadObj(*_assetManager, "Raziel.obj", razielFileContents);
    // razielData.model =
    //     glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f))
    //     * glm::scale(glm::mat4(1.0f), glm::vec3(3.0f, 3.0f, 3.0f));
    // razielData.diffuseTexture = {
    //   _assetManager->loadImageAsync(TEXTURES_PATH "Raziel.png"), TEXTURES_PATH "Raziel.png"};
    // const AssetManager::ImageData& razielImageData =
    //     _assetManager->getImageData(razielData.diffuseTexture.ID);

    // Texture razielTexture =
    //     createTexture2D(*_logicalDevice, commandBuffer, razielImageData,
    //     VK_FORMAT_R8G8B8A8_SRGB,
    //                     _physicalDevice->getMaxSamplerAnisotropy());
    //_razielEntity = loadObject(commandBuffer, razielData, _blinnPhongTesselationPipelineHandle,
    //                            std::move(razielTexture));
    //_objects.push_back(Object("Raziel", _razielEntity));
  }

  createOctreeScene();
  {
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
    recordShadowCommandBuffer(handle.getCommandBuffer());
    recordEnvMappingCommandBuffer(handle.getCommandBuffer());
  }
}

GCONTEXT_TEMPLATE
Entity GCONTEXT_CLASS loadObject(VkCommandBuffer commandBuffer, const common::VertexData& cubeData,
                                 PipelineHandle pipelineHandle, Texture&& texture) {
  Entity entity = _registry.createEntity();

  Sampler sampler = SamplerBuilder()
                        .withAnisotropy(_physicalDevice->getMaxSamplerAnisotropy())
                        .build(*_logicalDevice);
  _registry.addComponent<MaterialComponent>(
      entity, MaterialComponent{
                .diffuse = _bindlessWriter->writeTexture(
                    texture.getVkImageView(), texture.getVkImageLayout(), sampler.getVkSampler()),
                .pipelineHandle = pipelineHandle});
  _samplerManager->transferSampler(std::move(sampler));
  _gpuBufferManager->transferTexture(std::move(texture));

  MeshComponent msh = {.aabb = createAABBfromVertices(cubeData.positions, glm::mat4(1.0f))};
  if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    AssetManager::VertexData vData = _assetManager->releaseVertexData(cubeData.vertexResourceID);
    msh.vertexBufferPrimitiveHandle =
        _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("P")));
    msh.vertexBufferHandle = _gpuBufferManager->transferBuffer(std::move(vData.buffers.at("PN")));
    msh.indexBufferHandle = _gpuBufferManager->transferBuffer(std::move(vData.indexBuffer));
    msh.indexType = vData.indexType;
  } else if (_physicalDevice->getPhysicalDeviceType() == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    const AssetManager::VertexData& vData = _assetManager->getVertexData(cubeData.vertexResourceID);
    msh.vertexBufferPrimitiveHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.buffers.at("P"), GpuBufferManager::BufferType::VERTEX);
    msh.vertexBufferHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.buffers.at("PTN"), GpuBufferManager::BufferType::VERTEX);
    msh.indexBufferHandle = _gpuBufferManager->uploadBuffer(
        commandBuffer, vData.indexBuffer, GpuBufferManager::BufferType::INDEX);
    msh.indexType = vData.indexType;
  }
  _registry.addComponent(entity, std::move(msh));
  _registry.addComponent(entity, TransformComponent{.model = cubeData.model});
  return entity;
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createDescriptorSets() {
  // If VR presentation is enabled then multiply times 2 otherwise times 1.
  const uint32_t size =
      _logicalDevice->getPhysicalDevice().getMemoryAlignment(sizeof(UniformBufferCamera));
  _dynamicUniformBuffersCamera =
      BufferBuilder()
          .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
          .withSize((MULTIVIEW_PRESENTATION ? 2 : 1) * MAX_FRAMES_IN_FLIGHT * size)
          .createUniformBuffer(*_logicalDevice);

  _dynamicDescriptorSetWriter.storeDynamicBuffer(
      _dynamicUniformBuffersCamera.first, _dynamicUniformBuffersCamera.second.usage, size,
      MULTIVIEW_PRESENTATION ? 2 : 1);
  _dynamicDescriptorSetWriter.writeDescriptorSet(
      _logicalDevice->getVkDevice(), _dynamicDescriptorSet.getVkDescriptorSet());

  BufferWithMetadata lightBuffer = BufferBuilder()
                     .withUsage(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
                     .withSize(sizeof(UniformBufferLight))
                     .createUniformBuffer(*_logicalDevice);
  _lightHandle = _bindlessWriter->writeBuffer(lightBuffer);

  _ubLight.pos = glm::vec3(15.1891f, 2.66408f, -0.841221f);
  _ubLight.projView = glm::perspective(glm::radians(120.0f), 1.0f, 0.1f, 40.0f);
  _ubLight.projView[1][1] = -_ubLight.projView[1][1];
  _ubLight.projView = _ubLight.projView
                      * glm::lookAt(_ubLight.pos, glm::vec3(-3.82383f, 3.66503f, 1.30751f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
  common::copyData(lightBuffer.second.getMappedMemoryAsSpan(), 0, _ubLight);
  _lightBuffer = std::move(lightBuffer.first);
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createEnvMappingResources() {
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

  RenderpassBuilder renderpassBuilder(attachmentLayout);
  renderpassBuilder.createSubpass().addOutputAttachment(0).addOutputAttachment(1);
  _envMappingRenderPass =
      renderpassBuilder.withMultiView({0b111111}, {0b111111})
          .addDependency(
              VK_SUBPASS_EXTERNAL, 0,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
          .build(*_logicalDevice);

  _envMappingFramebuffer =
      createFramebufferFromTextures(_envMappingRenderPass, _envMappingAttachments);

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

  // TODO:
  //_envMappingUniformBuffer = Buffer::createUniformBuffer(*_logicalDevice, sizeof(faceTransform));
  //common::copyData(_envMappingUniformBuffer.getMappedMemory(), 0, faceTransform);
  //_envMappingHandle = _bindlessWriter->writeBuffer(_envMappingUniformBuffer);
  //Sampler sampler = SamplerBuilder().withAnisotropy(samplerAnisotropy).build(*_logicalDevice);
  //_envMappingTextureHandle = _bindlessWriter->writeTexture(
  //    _envMappingAttachments[0].getVkImageView(), _envMappingAttachments[0].getVkImageLayout(),
  //    sampler.getVkSampler());
  //_samplerManager->transferSampler(std::move(sampler));
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createShadowResources() {
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
  _shadowHandle = _bindlessWriter->writeTexture(
      _shadowMap.getVkImageView(), _shadowMap.getVkImageLayout(), sampler.getVkSampler());
  _samplerManager->transferSampler(std::move(sampler));

  AttachmentLayout attachmentLayout;
  attachmentLayout.addShadowAttachment(
      VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  RenderpassBuilder builder(attachmentLayout);
  builder.createSubpass().addOutputAttachment(0);
  _shadowRenderPass = builder.build(*_logicalDevice);
  _shadowFramebuffer = createFramebufferFromTextures(_shadowRenderPass, std::span(&_shadowMap, 1));
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createGraphicsPipelines() {
  _graphicsPipelineHandle = _pipelineManager->createPBRProgram(_renderPass, MULTIVIEW_PRESENTATION);
  _graphicsPipeline = _pipelineManager->getPipeline(_graphicsPipelineHandle);
  _graphicsTesselationPipelineHandle =
      _pipelineManager->createPbrTesselationProgram(_renderPass, MULTIVIEW_PRESENTATION);
  _blinnPhongTesselationPipelineHandle =
      _pipelineManager->createBlinnPhongTesselationProgram(_renderPass, MULTIVIEW_PRESENTATION);
  _graphicsTesselationPipeline = _pipelineManager->getPipeline(_graphicsTesselationPipelineHandle);
  _skyboxPipeline =
      _pipelineManager->getPipeline(_pipelineManager->createSkyboxProgram(_renderPass));
  _phongEnvMappingPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createEnvMappingProgram(_renderPass, MULTIVIEW_PRESENTATION));
  _shadowPipeline =
      _pipelineManager->getPipeline(_pipelineManager->createShadowProgram(_shadowRenderPass));
  _envMappingPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createPbrEnvMappingProgram(_envMappingRenderPass));
  _fsrPipeline = _pipelineManager->getPipeline(
      _pipelineManager->createFragmentShadingRateProgram(*_logicalDevice));
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createCommandBuffers() {
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

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createSyncObjects() {
  static constexpr VkFenceCreateInfo fenceInfo = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT};
  for (VkFence& fence : _frameFences) {
    CHECK_VKCMD(vkCreateFence(_logicalDevice->getVkDevice(), &fenceInfo, nullptr, &fence),
                "Failed to create VkFence.");
  }
}

GCONTEXT_TEMPLATE
std::tuple<UniformTextureHandle, GpuTextureHandle> GCONTEXT_CLASS getOrLoadTexture(
    std::unordered_map<StagingImageDataResourceHandle,
                       std::pair<UniformTextureHandle, GpuTextureHandle>>& textureCache,
    StagingImageDataResourceHandle textureID, VkFormat format, VkCommandBuffer commandBuffer,
    float maxSamplerAnisotropy, SamplerHandle samplerHandle) {
  auto [it, inserted] = textureCache.try_emplace(textureID);

  if (!inserted) {
    _gpuBufferManager->increaseRefCount(it->second.second);
    return it->second;
  }

  const AssetManager::ImageData& imgData = _assetManager->getImageData(textureID);
  Texture texture =
      createTexture2D(*_logicalDevice, commandBuffer, imgData, format, maxSamplerAnisotropy);
  UniformTextureHandle handle = _bindlessWriter->writeTexture(
      texture.getVkImageView(), texture.getVkImageLayout(),
      _samplerManager->getSampler(samplerHandle).getVkSampler());
  const GpuTextureHandle index = _gpuBufferManager->transferTexture(std::move(texture));

  const auto result = std::make_tuple(handle, index);
  it->second = result;

  return result;
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS loadObjects(
    std::span<const common::VertexData> sceneData, PipelineHandle pipelineHandle) {
  const float maxSamplerAnisotropy = _physicalDevice->getMaxSamplerAnisotropy();

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

  for (const common::VertexData& sceneObject : sceneData) {
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
        e, MaterialComponent{diffuseHandle, normalHandle, metallicRoughnessHandle, pipelineHandle});
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

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createOctreeScene() {
  if (_objects.empty()) {
    return;
  }

  AABB sceneAABB = _registry.getComponent<MeshComponent>(_objects[0].getEntity()).aabb;

  for (int i = 1; i < _objects.size(); ++i) {
    sceneAABB.extend(_registry.getComponent<MeshComponent>(_objects[i].getEntity()).aabb);
  }
  _octree = std::make_unique<Octree>(sceneAABB);

  for (const Object& object : _objects) {
    _octree->addObject(&object, _registry.getComponent<MeshComponent>(object.getEntity()).aabb);
  }
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS recordShadowCommandBuffer(VkCommandBuffer commandBuffer) {
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
                       _shadowPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

    VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(meshComponent.vertexBufferPrimitiveHandle).first.getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    const BufferWithMetadata& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.first.getVkBuffer(), 0, meshComponent.indexType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.second.size / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(commandBuffer);
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS recordEnvMappingCommandBuffer(VkCommandBuffer commandBuffer) {
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
        _envMappingPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

    VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle).first.getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

    const BufferWithMetadata& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.first.getVkBuffer(), 0, meshComponent.indexType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.second.size / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
  }

  vkCmdEndRenderPass(commandBuffer);
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS updateUniformBuffer(uint32_t currentFrame) {
  UniformBufferCamera _ubCamera;
  std::span<const common::CameraContext> cameraContexts = _communicationLayer->getCameraContexts();
  // TODO: Switch to std::views::enumerate.
  for (size_t i = 0; i < cameraContexts.size(); i++) {
    const common::CameraContext& cameraContext = cameraContexts[i];
    _ubCamera.view = cameraContext.view;
    _ubCamera.proj = cameraContext.proj;
    _ubCamera.pos = cameraContext.position;
    _ubCamera.viewDir = cameraContext.viewDir;
    common::copyData(_dynamicUniformBuffersCamera.second.getMappedMemoryAsSpan(),
                     (cameraContexts.size() * currentFrame + i)
                         * _physicalDevice->getMemoryAlignment(sizeof(_ubCamera)),
                     _ubCamera);
  }
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS recordOctreeSecondaryCommandBuffer(
    const VkCommandBuffer commandBuffer, const OctreeNode* rootNode,
    std::span<const glm::vec4> planes, std::span<VkDescriptorSet> descriptorSets,
    std::span<uint32_t> dynamicUniformBufferOffsets) {
  if (!rootNode || !rootNode->getVolume().intersectsFrustum(planes)) {
    return;
  }

  std::optional<PipelineHandle> globalPipelineHandle;
  Pipeline* pipeline;
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

      if (!globalPipelineHandle.has_value()
          || materialComponent.pipelineHandle != *globalPipelineHandle) {
        globalPipelineHandle = materialComponent.pipelineHandle;
        pipeline = _pipelineManager->getPipeline(*globalPipelineHandle);
        vkCmdBindPipeline(
            commandBuffer, pipeline->getVkPipelineBindPoint(), pipeline->getVkPipeline());
        vkCmdBindDescriptorSets(
            commandBuffer, pipeline->getVkPipelineBindPoint(), pipeline->getVkPipelineLayout(), 0,
            static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
            static_cast<uint32_t>(dynamicUniformBufferOffsets.size()),
            dynamicUniformBufferOffsets.data());
      }

      vkCmdPushConstants(commandBuffer, pipeline->getVkPipelineLayout(),
                         pipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

      const auto& meshComponent = _registry.getComponent<MeshComponent>(object->getEntity());
      const BufferWithMetadata& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
      const Buffer& vertexBuffer = _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle).first;
      static constexpr VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.getVkBuffer(), offsets);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer.first.getVkBuffer(), 0, meshComponent.indexType);
      vkCmdDrawIndexed(
          commandBuffer, indexBuffer.second.size / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
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

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS recordCommandBuffer(const glm::mat4& cameraProj, const glm::mat4& cameraView,
                                        uint32_t imageIndex, glm::u32vec2 screenPos) {
  const Framebuffer& framebuffer = _framebuffers[imageIndex];
  const CommandBuffer& primaryCommandBuffer = _primaryCommandBuffer[_currentFrame];
  primaryCommandBuffer.beginAsPrimary();

  const VkCommandBuffer commandBuffer = primaryCommandBuffer.getVkCommandBuffer();

  const PushConstantFov fsrPc = {screenPos};
  vkCmdPushConstants(commandBuffer, _fsrPipeline->getVkPipelineLayout(),
                     VK_SHADER_STAGE_COMPUTE_BIT, 0, 8, &fsrPc);
  vkCmdBindPipeline(
      commandBuffer, _fsrPipeline->getVkPipelineBindPoint(), _fsrPipeline->getVkPipeline());
  const VkDescriptorSet fsrDescriptorSets[] = {_computeDescriptorSet.getVkDescriptorSet()};
  vkCmdBindDescriptorSets(commandBuffer, _fsrPipeline->getVkPipelineBindPoint(),
                          _fsrPipeline->getVkPipelineLayout(), 0, 1, fsrDescriptorSets, 0, nullptr);
  vkCmdDispatch(commandBuffer, 16, 16, 1);

  const Texture& fsrTexture = _gpuBufferManager->getTexture(_fsrTextureHandle);
  const VkImageMemoryBarrier fsrBarrier = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
    .srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT,
    .dstAccessMask = VK_ACCESS_FRAGMENT_SHADING_RATE_ATTACHMENT_READ_BIT_KHR,
    .oldLayout = VK_IMAGE_LAYOUT_GENERAL,
    .newLayout = VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = fsrTexture.getVkImage(),
    .subresourceRange = {
                         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                         .baseMipLevel = 0,
                         .levelCount = fsrTexture.getMipLevelsCount(),
                         .baseArrayLayer = 0,
                         .layerCount = fsrTexture.getLayersCount(),
                         }
  };
  vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                       VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR, 0, 0, nullptr, 0,
                       nullptr, 1, &fsrBarrier);

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
        _secondaryCommandBuffers[0][_currentFrame].getVkCommandBuffer();

    if (viewportScissorInheritance) [[likely]] {
      _secondaryCommandBuffers[0][_currentFrame].beginAsSecondary(
          framebuffer, &scissorViewportInheritance);
    } else {
      _secondaryCommandBuffers[0][_currentFrame].beginAsSecondary(framebuffer, nullptr);
      vkCmdSetViewport(commandBuffer, 0, 1, &framebuffer.getViewport());
      vkCmdSetScissor(commandBuffer, 0, 1, &framebuffer.getScissor());
    }

    // vkCmdBindPipeline(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(),
    //                   _graphicsPipeline->getVkPipeline());

    const OctreeNode* root = _octree->getRoot();
    const auto& planes = extractFrustumPlanes(cameraProj * cameraView);

    VkDescriptorSet descriptorSets[] = {
      _bindlessDescriptorSet.getVkDescriptorSet(), _dynamicDescriptorSet.getVkDescriptorSet()};

    if constexpr (MULTIVIEW_PRESENTATION) {
      uint32_t dynamicUniformBufferOffsets[2];
      const uint32_t baseOffset = 2u * _currentFrame;
      _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
          dynamicUniformBufferOffsets, {baseOffset, baseOffset});
      recordOctreeSecondaryCommandBuffer(
          commandBuffer, root, planes, descriptorSets, dynamicUniformBufferOffsets);
    } else {
      uint32_t dynamicUniformBufferOffset[1];
      _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
          dynamicUniformBufferOffset, {_currentFrame});
      recordOctreeSecondaryCommandBuffer(
          commandBuffer, root, planes, descriptorSets, dynamicUniformBufferOffset);
    }

    CHECK_VKCMD(vkEndCommandBuffer(commandBuffer), "Failed to vkEndCommandBuffer.");
  });

  futures[1] = std::async(std::launch::async, [&]() -> void {
    // Skybox
    const VkCommandBuffer commandBuffer =
        _secondaryCommandBuffers[1][_currentFrame].getVkCommandBuffer();

    if (viewportScissorInheritance) [[likely]] {
      _secondaryCommandBuffers[1][_currentFrame].beginAsSecondary(
          framebuffer, &scissorViewportInheritance);
    } else {
      _secondaryCommandBuffers[1][_currentFrame].beginAsSecondary(framebuffer, nullptr);
      vkCmdSetViewport(commandBuffer, 0, 1, &framebuffer.getViewport());
      vkCmdSetScissor(commandBuffer, 0, 1, &framebuffer.getScissor());
    }

    vkCmdBindPipeline(
        commandBuffer, _skyboxPipeline->getVkPipelineBindPoint(), _skyboxPipeline->getVkPipeline());

    static constexpr VkDeviceSize offsets[] = {0};

    const MeshComponent& cubeMeshComponent = _registry.getComponent<MeshComponent>(_skyboxEntity);
    const MaterialComponent& cubeMaterialComponent =
        _registry.getComponent<MaterialComponent>(_skyboxEntity);
    const VkBuffer vertexBuffer =
        _gpuBufferManager->getBuffer(cubeMeshComponent.vertexBufferPrimitiveHandle).first.getVkBuffer();
    const BufferWithMetadata& indexBuffer = _gpuBufferManager->getBuffer(cubeMeshComponent.indexBufferHandle);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.first.getVkBuffer(), 0, cubeMeshComponent.indexType);

    const PushConstantsSkybox pc = {
      .proj = cameraProj,
      .view = cameraView,
      .skyboxHandle = static_cast<uint32_t>(*cubeMaterialComponent.diffuse)};
    vkCmdPushConstants(commandBuffer, _skyboxPipeline->getVkPipelineLayout(),
                       _skyboxPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

    const VkDescriptorSet descriptorSets[] = {
      _bindlessDescriptorSet.getVkDescriptorSet(), _dynamicDescriptorSet.getVkDescriptorSet()};

    vkCmdBindDescriptorSets(
        commandBuffer, _skyboxPipeline->getVkPipelineBindPoint(),
        _skyboxPipeline->getVkPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);

    vkCmdDrawIndexed(commandBuffer,
                     indexBuffer.second.size / getIndexSize(cubeMeshComponent.indexType), 1, 0, 0, 0);

    // Env mapping
    /*vkCmdBindPipeline(commandBuffer, _phongEnvMappingPipeline->getVkPipelineBindPoint(),
                      _phongEnvMappingPipeline->getVkPipeline());

    uint32_t dynamicUniformBufferOffsets[MULTIVIEW_PRESENTATION ? 2 : 1];
    if constexpr (MULTIVIEW_PRESENTATION) {
      const uint32_t baseOffset = 2u * _currentFrame;
      _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
          dynamicUniformBufferOffsets, {baseOffset, baseOffset});
    } else {
      _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
          dynamicUniformBufferOffsets, {_currentFrame});
    }
    vkCmdBindDescriptorSets(
        commandBuffer, _phongEnvMappingPipeline->getVkPipelineBindPoint(),
        _phongEnvMappingPipeline->getVkPipelineLayout(), 0, std::size(descriptorSets),
        descriptorSets, std::size(dynamicUniformBufferOffsets), dynamicUniformBufferOffsets);

    const PushConstantsModelDescriptorHandles32Bit envMapPc = {
      .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))
               * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)),
      .descriptorHandles = {
                            static_cast<uint32_t>(*_envMappingHandle),
    static_cast<uint32_t>(*_lightHandle)}
    };

    vkCmdPushConstants(commandBuffer, _phongEnvMappingPipeline->getVkPipelineLayout(),
                       _phongEnvMappingPipeline->getPushConstantVkShaderStageFlags(), 0,
                       sizeof(envMapPc), &envMapPc);

    const VkBuffer vertexBufferCubeNormals =
        _gpuBufferManager->getBuffer(_vertexBufferCubeNormalsHandle).getVkBuffer();
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBufferCubeNormals, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, _indexBufferCubeType);

    vkCmdDrawIndexed(
        commandBuffer, indexBuffer.getSize() / getIndexSize(_indexBufferCubeType), 1, 0, 0, 0);*/

    CHECK_VKCMD(vkEndCommandBuffer(commandBuffer), "Failed to vkEndCommandBuffer.");
  });

  std::for_each(std::begin(futures), std::end(futures), [](std::future<void>& future) {
    future.wait();
  });

  primaryCommandBuffer.executeSecondaryCommandBuffers(
      {_secondaryCommandBuffers[0][_currentFrame].getVkCommandBuffer(),
       _secondaryCommandBuffers[1][_currentFrame].getVkCommandBuffer()});
  primaryCommandBuffer.endRenderPass();

  if (primaryCommandBuffer.end() != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}

GCONTEXT_TEMPLATE
GCONTEXT_CLASS GraphicsContext(
    std::shared_ptr<Instance> instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice> physicalDevice, std::unique_ptr<LogicalDevice> logicalDevice,
    const FileLoader& fileLoader,
    std::shared_ptr<engine::PresentationGraphicsCommunication> communicationLayer,
    std::unique_ptr<PresentationContext> presentationContext)
  : _instance(std::move(instance)), _debugMessenger(std::move(debugMessenger)),
    _physicalDevice(std::move(physicalDevice)), _logicalDevice(std::move(logicalDevice)),
    _fileLoader(fileLoader), _communicationLayer(std::move(communicationLayer)),
    _presentationContext(std::move(presentationContext)),
    _singleTimeCommandPool(
        CommandPool::create(*_logicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)),
    _assetManager(AssetManager::create(*_logicalDevice, fileLoader, std::launch::async)),
    _gpuBufferManager(GpuBufferManager::create()), _samplerManager(SamplerManager::create()),
    _pipelineManager(PipelineManager::create(fileLoader)),
    _framebufferAttachmentManager(
        std::make_unique<FramebufferAttachmentManager>(*_gpuBufferManager)),
    _bindlessDescriptorPool(DescriptorPool::create(
        *_logicalDevice, 1, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)),
    _bindlessDescriptorSet(_bindlessDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateBindlessLayout(*_logicalDevice))),
    _bindlessWriter(BindlessDescriptorSetWriter::create(_bindlessDescriptorSet)),
    _dynamicDescriptorPool(DescriptorPool::create(*_logicalDevice, 1)),
    _dynamicDescriptorSet(_dynamicDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateCameraLayout(*_logicalDevice, MULTIVIEW_PRESENTATION))),
    _computeDescriptorPool(DescriptorPool::create(*_logicalDevice, 1)),
    _computeDescriptorSet(_computeDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateComputeLayout(*_logicalDevice))) {}

GCONTEXT_TEMPLATE
std::unique_ptr<common::GraphicsContext> GCONTEXT_CLASS create(
    std::shared_ptr<Instance> instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice> physicalDevice, std::unique_ptr<LogicalDevice> logicalDevice,
    const FileLoader& fileLoader,
    std::shared_ptr<engine::PresentationGraphicsCommunication> communicationLayer,
    std::unique_ptr<PresentationContext> presentationContext) {
  return std::unique_ptr<GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>>(
      new GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>(
          std::move(instance), std::move(debugMessenger), std::move(physicalDevice),
          std::move(logicalDevice), fileLoader, std::move(communicationLayer),
          std::move(presentationContext)));
}

GCONTEXT_TEMPLATE
GCONTEXT_CLASS ~GraphicsContext() {
  const VkDevice device = _logicalDevice->getVkDevice();

  vkWaitForFences(device, MAX_FRAMES_IN_FLIGHT, _frameFences.data(), VK_TRUE, UINT64_MAX);
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroyFence(device, _frameFences[i], nullptr);
  }
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS draw() {
  updateUniformBuffer(_currentFrame);

  _primaryCommandBuffer[_currentFrame].resetCommandBuffer();
  for (int i = 0; i < MAX_THREADS_IN_POOL; i++) {
    _secondaryCommandBuffers[i][_currentFrame].resetCommandBuffer();
  }

  vkResetFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_currentFrame]);

  const common::CameraContext& cameraContext = _communicationLayer->getCameraContexts()[0];
  const auto [screenx, screeny] = _communicationLayer->getScreenPos();
  recordCommandBuffer(cameraContext.proj, cameraContext.view,
                      _communicationLayer->getCurrentSwapchainImageIndex(), {screenx, screeny});

  VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.pWaitDstStageMask = waitStages;

  VkCommandBuffer submitCommands[] = {_primaryCommandBuffer[_currentFrame].getVkCommandBuffer()};
  submitInfo.commandBufferCount = static_cast<uint32_t>(std::size(submitCommands));
  submitInfo.pCommandBuffers = submitCommands;

  if constexpr (!SYNCED_OUTSIDE) {
    _presentationContext->synchronizeSubmit(&submitInfo);
  }

  if (vkQueueSubmit(
          _logicalDevice->getGraphicsVkQueue(), 1, &submitInfo, _frameFences[_currentFrame])
      != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  if (++_currentFrame == MAX_FRAMES_IN_FLIGHT) {
    _currentFrame = 0;
  }

  if constexpr (!SYNCED_OUTSIDE) {
    _presentationContext->setCurrentFrame(_currentFrame);
  }
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS initializeResources() {
  setup();
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS waitCompleteExecution() const {
  vkWaitForFences(
      _logicalDevice->getVkDevice(), 1, &_frameFences[_currentFrame], VK_TRUE, UINT64_MAX);
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS createPresentingResources(const common::PresentResources& presentResources) {
  lib::Buffer<VkPhysicalDeviceFragmentShadingRateKHR> fragmentShadingRates =
      _physicalDevice->getFragmentShadingRates();

  static constexpr VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_2_BIT;
  const VkFormat swapchainImageFormat = static_cast<VkFormat>(presentResources.imageFormat);
  const VkExtent2D extent = VkExtent2D{presentResources.width, presentResources.height};

  AttachmentLayout attachmentsLayout(msaaSamples);
  attachmentsLayout
      .addColorResolvePresentAttachment(swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE)
      .addColorAttachment(
          swapchainImageFormat, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE)
      .addDepthAttachment(VK_FORMAT_D24_UNORM_S8_UINT, VK_ATTACHMENT_STORE_OP_DONT_CARE)
      .addFragmentShadingRateAttachment();

  lib::Buffer<GpuTextureHandle> attachmentHandles;
  {
    SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
    const VkCommandBuffer commandBuffer = handle.getCommandBuffer();
    GpuTextureHandle collorAttachmentHandle = _gpuBufferManager->transferTexture(createAttachment(
        *_logicalDevice, commandBuffer, swapchainImageFormat, msaaSamples, extent,
        presentResources.numLayers, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT));
    GpuTextureHandle depthAttachmentHandle = _gpuBufferManager->transferTexture(createAttachment(
        *_logicalDevice, commandBuffer, VK_FORMAT_D24_UNORM_S8_UINT, msaaSamples, extent,
        presentResources.numLayers, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT));

    const VkPhysicalDeviceFragmentShadingRatePropertiesKHR& fsrProperties =
        _physicalDevice->getFragmentShadingRateProperties();
    const VkExtent2D fsrTexelExtent = fsrProperties.maxFragmentShadingRateAttachmentTexelSize;
    const VkExtent2D fsrExtent = VkExtent2D{
      static_cast<uint32_t>(std::ceil(extent.width / static_cast<float>(fsrTexelExtent.width))),
      static_cast<uint32_t>(std::ceil(extent.height / static_cast<float>(fsrTexelExtent.height)))};
    Texture fsrTexture = createAttachment(
        *_logicalDevice, commandBuffer, VK_FORMAT_R8_UINT, VK_SAMPLE_COUNT_1_BIT, fsrExtent,
        presentResources.numLayers, VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_STORAGE_BIT);
    createFsrContents(fsrTexture, *_logicalDevice, *_singleTimeCommandPool);

    _computeDescriptorSetWriter.storeImageStorage(fsrTexture);
    _computeDescriptorSetWriter.writeDescriptorSet(
        _logicalDevice->getVkDevice(), _computeDescriptorSet.getVkDescriptorSet());

    GpuTextureHandle fsrAttachmentHandle = _fsrTextureHandle =
        _gpuBufferManager->transferTexture(std::move(fsrTexture));

    attachmentHandles = lib::Buffer<GpuTextureHandle>{
      collorAttachmentHandle, depthAttachmentHandle, fsrAttachmentHandle};
  }

  RenderpassBuilder renderpassBuilder(attachmentsLayout);
  if constexpr (MULTIVIEW_PRESENTATION) {
    auto mask = lib::setNLeastSignificantBits<uint32_t>(presentResources.numLayers);
    renderpassBuilder.withMultiView({mask}, {mask});
  }

  const VkExtent2D fsrTexelSize =
      _physicalDevice->getFragmentShadingRateProperties().maxFragmentShadingRateAttachmentTexelSize;
  renderpassBuilder.createSubpass()
      .addOutputAttachment(0)
      .addOutputAttachment(1)
      .addOutputAttachment(2)
      .withShadingRateAttachment(fsrTexelSize.width, fsrTexelSize.height);
  _renderPass =
      renderpassBuilder
          .addDependency(
              VK_SUBPASS_EXTERNAL, 0,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
              VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
                  | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
              VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
          .build(*_logicalDevice);

  auto imageViews = std::span<const VkImageView>(
      reinterpret_cast<const VkImageView*>(presentResources.imageViews.data()),
      presentResources.imageViews.size());
  for (VkImageView imageView : imageViews) {
    _framebuffers.push_back(_framebufferAttachmentManager->createFramebuffer(
        _renderPass, attachmentHandles, extent, imageView));
  }
}

GCONTEXT_TEMPLATE
void GCONTEXT_CLASS waitDeviceIdle() const {
  vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

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
          .buildImage(logicalDevice);
  texture.copyFromBuffer(
      commandBuffer, imageData.stagingBuffer.getVkBuffer(), imageData.copyRegions);
  texture.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice);
  texture.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice);
  texture.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice);
  texture.copyFromBuffer(
      commandBuffer, imageData.stagingBuffer.getVkBuffer(), imageData.copyRegions);
  texture.generateMipmaps(commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  texture.addCreateVkImageView(0, imageData.mipLevels, 0, 1);

  return texture;
}

Texture createAttachment(const LogicalDevice& logicalDevice, VkCommandBuffer commandBuffer,
                         VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent,
                         uint32_t numLayers, VkImageAspectFlags aspect, VkImageUsageFlags usage) {
  Texture texture =
      TextureBuilder()
          .withFormat(format)
          .withNumSamples(samples)
          .withExtent(extent)
          .withLayerCount(numLayers)
          .withAspect(aspect)
          .withUsage(usage)
          .buildImage(logicalDevice);
  texture.addCreateVkImageView(0, 1, 0, numLayers);
  return texture;
}

void createFsrContents(
    Texture& texture, const LogicalDevice& logicalDevice, const CommandPool& commandPool) {
  const VkExtent2D extent = texture.getVkExtent2D();
  const lib::Buffer<std::byte> buffer(
      static_cast<size_t>(extent.width * extent.height), std::byte{10});
  BufferBuilder bufferBuilder;
  auto [stagingBuffer, stagingBufferMetadata] = bufferBuilder
      .withSize(buffer.size())
      .withUsage(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
      .createStagingBuffer(logicalDevice);
  common::copyData(stagingBufferMetadata.getMappedMemoryAsSpan(), 0, std::span(buffer));
  lib::Buffer<VkBufferImageCopy> imageCopy(texture.getLayersCount());
  for (uint32_t layer = 0; layer < imageCopy.size(); layer++) {
    imageCopy[layer] = VkBufferImageCopy{
      .imageSubresource = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                           .mipLevel = 0,
                           .baseArrayLayer = layer,
                           .layerCount = 1},
      .imageExtent = VkExtent3D{extent.width, extent.height, 1},
    };
  }
  SingleTimeCommandBuffer handle(commandPool);
  texture.copyFromBuffer(handle.getCommandBuffer(), stagingBuffer.getVkBuffer(), imageCopy);
  texture.transitionLayout(handle.getCommandBuffer(), VK_IMAGE_LAYOUT_GENERAL);
}

}  // namespace

// Explicit instantiation of the specializations that are actually used.
// These must appear AFTER all member definitions above.
template class GraphicsContext<false, false>;

template class GraphicsContext<true, true>;

}  // namespace vlkn

#undef GCONTEXT_CLASS
#undef GCONTEXT_TEMPLATE
