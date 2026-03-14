#pragma once

#include <any>
#include <array>
#include <iostream>
#include <ranges>
#include <vector>

#include "common/abstractions/contexts.h"
#include "common/abstractions/graphics_context.h"
#include "common/entity_component_system/registry/registry.h"
#include "common/model_loader/model_loader.h"
#include "common/object/object.h"
#include "common/scene/octree.h"
#include "common/util/primitives.h"
#include "lib/bitwise.h"
#include "vulkan/graphics_context/graphics_context.h"
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
#include "vulkan/wrapper/instance/instance.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/physical_device/physical_device.h"

///
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/render_pass/render_pass.h"

///
#include "common/entity_component_system/component/material.h"
#include "common/entity_component_system/component/mesh.h"
#include "common/entity_component_system/component/transform.h"
#include "common/model_loader/obj_loader/obj_loader.h"
#include "common/model_loader/tiny_gltf_loader/tiny_gltf_loader.h"

inline VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback1(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
  std::cerr << "[Vulkan Validation] " << "Severity: " << messageSeverity << ", "
            << "Type: " << messageType << std::endl
            << "Message: " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

namespace vlkn {

constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;

struct SynchronizationContext {
  uint8_t currentFrame = 0;
  std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores;
  std::vector<VkSemaphore> renderFinishedSemaphores;
};

struct SwapchainContext {
  VkFormat imageFormat;
  VkExtent2D imageExtent;
  std::span<const VkImageView> imageViews;
  bool multiview;
};

namespace {

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
    VkCommandBuffer commandBuffer, Texture& texture, const LogicalDevice& logicalDevice);

}  // namespace

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
class GraphicsContext final : public common::GraphicsContext {
  GraphicsContext(std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
                  std::unique_ptr<PhysicalDevice>&& physicalDevice,
                  std::unique_ptr<LogicalDevice>&& logicalDevice, const FileLoader& fileLoader);

public:
  static std::unique_ptr<common::GraphicsContext> create(
      std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
      std::unique_ptr<PhysicalDevice>&& physicalDevice,
      std::unique_ptr<LogicalDevice>&& logicalDevice, const FileLoader& fileLoader);

  ~GraphicsContext();

  common::UpdateContextResponse update(const common::UpdateContext& updateContext) override;

  void draw(const common::DrawingContext& drawingContext) override;

  void initializeResources() override;

  void waitCompleteExecution() const override;

  void createPresentingResources(const common::PresentResources& presentResources) override;

  void waitDeviceIdle() const override;

  std::any getSynchronizationContext() const override;

  static constexpr uint32_t MAX_THREADS_IN_POOL = 2;

private:
  std::unique_ptr<Instance> _instance;
  DebugMessenger _debugMessenger;
  std::unique_ptr<PhysicalDevice> _physicalDevice;
  std::unique_ptr<LogicalDevice> _logicalDevice;

  const FileLoader& _fileLoader;

  SynchronizationContext _synchContext;

  std::array<VkFence, MAX_FRAMES_IN_FLIGHT> _frameFences;

  std::shared_ptr<CommandPool> _singleTimeCommandPool;

  std::unique_ptr<AssetManager> _assetManager;
  std::unique_ptr<GpuBufferManager> _gpuBufferManager;
  std::unique_ptr<SamplerManager> _samplerManager;
  std::unique_ptr<PipelineManager> _pipelineManager;
  std::unique_ptr<FramebufferAttachmentManager> _framebufferAttachmentManager;

  std::shared_ptr<DescriptorPool> _bindlessDescriptorPool;
  DescriptorSet _bindlessDescriptorSet;
  std::unique_ptr<BindlessDescriptorSetWriter> _bindlessWriter;

  std::shared_ptr<DescriptorPool> _dynamicDescriptorPool;
  DescriptorSet _dynamicDescriptorSet;
  DescriptorSetWriter _dynamicDescriptorSetWriter;

  std::shared_ptr<DescriptorPool> _computeDescriptorPool;
  DescriptorSet _computeDescriptorSet;
  DescriptorSetWriter _computeDescriptorSetWriter;

  // Temporal objects for demonstration:
  std::array<std::shared_ptr<CommandPool>, MAX_THREADS_IN_POOL + 1> _commandPools;
  std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT> _primaryCommandBuffer;
  std::array<std::array<CommandBuffer, MAX_FRAMES_IN_FLIGHT>, MAX_THREADS_IN_POOL>
      _secondaryCommandBuffers;

  std::vector<Object> _objects;
  std::unique_ptr<Octree> _octree;
  Registry _registry;

  Renderpass _renderPass;
  std::vector<Framebuffer> _framebuffers;

  // Shadowmap
  Renderpass _shadowRenderPass;
  Framebuffer _shadowFramebuffer;
  Texture _shadowMap;
  Pipeline* _shadowPipeline;
  UniformTextureHandle _shadowHandle;

  // Skybox.
  GpuBufferHandle _vertexBufferCubeHandle;
  GpuBufferHandle _vertexBufferCubeNormalsHandle;
  GpuBufferHandle _indexBufferCubeHandle;
  Texture _textureCubemap;
  VkIndexType _indexBufferCubeType;
  Pipeline* _skyboxPipeline;
  UniformTextureHandle _skyboxHandle;

  // Mirror cubemap
  // First pass.
  Renderpass _envMappingRenderPass;
  Framebuffer _envMappingFramebuffer;
  Pipeline* _envMappingPipeline;
  Buffer _envMappingUniformBuffer;
  UniformBufferHandle _envMappingHandle;
  std::array<Texture, 2> _envMappingAttachments;
  UniformTextureHandle _envMappingTextureHandle;
  // Second pass.
  Pipeline* _phongEnvMappingPipeline;

  // PBR objects.
  std::vector<Object> objects;
  Pipeline* _graphicsPipeline;

  UniformBufferLight _ubLight;
  Buffer _dynamicUniformBuffersCamera;
  Buffer _lightBuffer;
  UniformBufferHandle _lightHandle;

  // Fragment rate shading.
  Pipeline* _fsrPipeline;
  GpuTextureHandle _fsrTextureHandle;

  void setup() {
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

  void loadCubemap(const VertexData& cubeData) {
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
      const AssetManager::VertexData& vData =
          _assetManager->getVertexData(cubeData.vertexResourceID);
      _vertexBufferCubeHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.buffers.at("P"), GpuBufferManager::BufferType::VERTEX);
      _vertexBufferCubeNormalsHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.buffers.at("PN"), GpuBufferManager::BufferType::VERTEX);
      _indexBufferCubeHandle = _gpuBufferManager->uploadBuffer(
          commandBuffer, vData.indexBuffer, GpuBufferManager::BufferType::INDEX);
      _indexBufferCubeType = vData.indexType;
    }
  }

  void createDescriptorSets() {
    // If VR presentation is enabled then multiply times 2 otherwise times 1.
    const uint32_t size =
        _logicalDevice->getPhysicalDevice().getMemoryAlignment(sizeof(UniformBufferCamera));
    _dynamicUniformBuffersCamera = Buffer::createUniformBuffer(
        *_logicalDevice, (MULTIVIEW_PRESENTATION ? 2 : 1) * MAX_FRAMES_IN_FLIGHT * size);
    Sampler sampler = SamplerBuilder()
                          .withAnisotropy(_physicalDevice->getMaxSamplerAnisotropy())
                          .build(*_logicalDevice);
    _skyboxHandle = _bindlessWriter->storeTexture(_textureCubemap, sampler);
    _samplerManager->transferSampler(std::move(sampler));

    _dynamicDescriptorSetWriter.storeDynamicBuffer(
        _dynamicUniformBuffersCamera, size, MULTIVIEW_PRESENTATION ? 2 : 1);
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

  void createEnvMappingResources() {
    // First pass for rendering the environment map.
    const float samplerAnisotropy = _physicalDevice->getMaxSamplerAnisotropy();
    {
      SingleTimeCommandBuffer handle(*_singleTimeCommandPool);

      _envMappingAttachments[0] = createCubemap(
          *_logicalDevice, handle.getCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT,
          VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, samplerAnisotropy);
      _envMappingAttachments[1] = createCubemap(
          *_logicalDevice, handle.getCommandBuffer(), VK_IMAGE_ASPECT_DEPTH_BIT,
          VK_FORMAT_D16_UNORM, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, samplerAnisotropy);
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
                     proj
                * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                     proj
                * glm::lookAt(
                    pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                     proj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
                     proj
                * glm::lookAt(
                    pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
                     proj
                * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
                     proj
                * glm::lookAt(
                    pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
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

  void createShadowResources() {
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

    RenderpassBuilder builder(attachmentLayout);
    builder.createSubpass().addOutputAttachment(0);
    _shadowRenderPass = builder.build(*_logicalDevice);
    _shadowFramebuffer =
        Framebuffer::createFromTextures(_shadowRenderPass, std::span(&_shadowMap, 1));
  }

  void createGraphicsPipelines() {
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

    _fsrPipeline = _pipelineManager->getPipeline(
        _pipelineManager->createFragmentShadingRateProgram(*_logicalDevice));
  }

  void createCommandBuffers() {
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

  void createSyncObjects() {
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
      CHECK_VKCMD(
          vkCreateFence(_logicalDevice->getVkDevice(), &fenceInfo, nullptr, &_frameFences[i]),
          "Failed to create VkFence.");
    }
  }

  std::tuple<UniformTextureHandle, GpuTextureHandle> getOrLoadTexture(
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

  void loadObjects(std::span<const VertexData> sceneData) {
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

  void createOctreeScene() {
    AABB sceneAABB = _registry.getComponent<MeshComponent>(_objects[0].getEntity()).aabb;

    for (int i = 1; i < _objects.size(); ++i) {
      sceneAABB.extend(_registry.getComponent<MeshComponent>(_objects[i].getEntity()).aabb);
    }
    _octree = std::make_unique<Octree>(sceneAABB);

    for (const Object& object : _objects) {
      _octree->addObject(&object, _registry.getComponent<MeshComponent>(object.getEntity()).aabb);
    }
  }

  void recordShadowCommandBuffer(VkCommandBuffer commandBuffer) {
    const VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

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
      const auto& transformComponent =
          _registry.getComponent<TransformComponent>(object.getEntity());

      pc.model = transformComponent.model;

      vkCmdPushConstants(commandBuffer, _shadowPipeline->getVkPipelineLayout(),
                         _shadowPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

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

  void recordEnvMappingCommandBuffer(VkCommandBuffer commandBuffer) {
    const VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

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
      const auto& transformComponent =
          _registry.getComponent<TransformComponent>(object.getEntity());
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
          _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle).getVkBuffer();
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);

      const Buffer& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, meshComponent.indexType);

      vkCmdDrawIndexed(
          commandBuffer, indexBuffer.getSize() / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);
  }

  void updateUniformBuffer(
      const std::vector<common::CameraContext>& cameraContexts, uint32_t currentFrame) {
    if constexpr (MULTIVIEW_PRESENTATION) {
      UniformBufferCamera _ubCamera;
      for (size_t i = 0; i < 2; ++i) {
        _ubCamera.view = cameraContexts[i].view;
        _ubCamera.proj = cameraContexts[i].proj;
        _ubCamera.pos = cameraContexts[i].position;
        _dynamicUniformBuffersCamera.copyData(
            _ubCamera, (2 * currentFrame + i)
                           * _physicalDevice->getMemoryAlignment(sizeof(UniformBufferCamera)));
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

  void recordOctreeSecondaryCommandBuffer(
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
            _graphicsPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

        const auto& meshComponent = _registry.getComponent<MeshComponent>(object->getEntity());
        const Buffer& indexBuffer = _gpuBufferManager->getBuffer(meshComponent.indexBufferHandle);
        const Buffer& vertexBuffer = _gpuBufferManager->getBuffer(meshComponent.vertexBufferHandle);
        static constexpr VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.getVkBuffer(), offsets);
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, meshComponent.indexType);
        vkCmdDrawIndexed(commandBuffer,
                         indexBuffer.getSize() / getIndexSize(meshComponent.indexType), 1, 0, 0, 0);
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

  void recordCommandBuffer(const glm::mat4& cameraProj, const glm::mat4& cameraView,
                           uint32_t imageIndex, glm::u32vec2 screenPos) {
    const Framebuffer& framebuffer = _framebuffers[imageIndex];
    const CommandBuffer& primaryCommandBuffer = _primaryCommandBuffer[_synchContext.currentFrame];
    primaryCommandBuffer.beginAsPrimary();

    const VkCommandBuffer commandBuffer = primaryCommandBuffer.getVkCommandBuffer();

    const PushConstantFov fsrPc = {
        screenPos
    };
    vkCmdPushConstants(commandBuffer, _fsrPipeline->getVkPipelineLayout(),
                       VK_SHADER_STAGE_COMPUTE_BIT, 0, 8, &fsrPc);
    vkCmdBindPipeline(
        commandBuffer, _fsrPipeline->getVkPipelineBindPoint(),
                      _fsrPipeline->getVkPipeline());
    const VkDescriptorSet fsrDescriptorSets[] = {_computeDescriptorSet.getVkDescriptorSet()};
    vkCmdBindDescriptorSets(
        commandBuffer, _fsrPipeline->getVkPipelineBindPoint(),
        _fsrPipeline->getVkPipelineLayout(), 0, 1, fsrDescriptorSets, 0, nullptr);
    vkCmdDispatch(commandBuffer, 16, 16, 1);

    const Texture& fsrTexture = _gpuBufferManager->getTexture(_fsrTextureHandle);
    VkImageMemoryBarrier fsrBarrier = {
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
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }};
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR,
                         0, 0, nullptr, 0, nullptr, 1, &fsrBarrier);

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

      uint32_t dynamicUniformBufferOffsets[MULTIVIEW_PRESENTATION ? 2 : 1];
      if constexpr (MULTIVIEW_PRESENTATION) {
        const uint32_t baseOffset = 2u * _synchContext.currentFrame;
        _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
            dynamicUniformBufferOffsets, {baseOffset, baseOffset});
      } else {
        _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
            dynamicUniformBufferOffsets, {_synchContext.currentFrame});
      }
      vkCmdBindDescriptorSets(commandBuffer, _graphicsPipeline->getVkPipelineBindPoint(),
                              _graphicsPipeline->getVkPipelineLayout(), 0,
                              static_cast<uint32_t>(std::size(descriptorSets)), descriptorSets,
                              std::size(dynamicUniformBufferOffsets), dynamicUniformBufferOffsets);

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

      vkCmdBindPipeline(commandBuffer, _skyboxPipeline->getVkPipelineBindPoint(),
                        _skyboxPipeline->getVkPipeline());

      static constexpr VkDeviceSize offsets[] = {0};

      const VkBuffer vertexBuffer =
          _gpuBufferManager->getBuffer(_vertexBufferCubeHandle).getVkBuffer();
      const Buffer& indexBuffer = _gpuBufferManager->getBuffer(_indexBufferCubeHandle);
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
      vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getVkBuffer(), 0, _indexBufferCubeType);

      const PushConstantsSkybox pc = {.proj = cameraProj,
                                      .view = cameraView,
                                      .skyboxHandle = static_cast<uint32_t>(*_skyboxHandle)};
      vkCmdPushConstants(commandBuffer, _skyboxPipeline->getVkPipelineLayout(),
                         _skyboxPipeline->getPushConstantVkShaderStageFlags(), 0, sizeof(pc), &pc);

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

      uint32_t dynamicUniformBufferOffsets[MULTIVIEW_PRESENTATION ? 2 : 1];
      if constexpr (MULTIVIEW_PRESENTATION) {
        const uint32_t baseOffset = 2u * _synchContext.currentFrame;
        _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
            dynamicUniformBufferOffsets, {baseOffset, baseOffset});
      } else {
        _dynamicDescriptorSetWriter.getDynamicBufferSizesWithOffsets(
            dynamicUniformBufferOffsets, {_synchContext.currentFrame});
      }
      vkCmdBindDescriptorSets(
          commandBuffer, _phongEnvMappingPipeline->getVkPipelineBindPoint(),
          _phongEnvMappingPipeline->getVkPipelineLayout(), 0, std::size(descriptorSets),
          descriptorSets, std::size(dynamicUniformBufferOffsets), dynamicUniformBufferOffsets);

      const PushConstantsModelDescriptorHandles32Bit envMapPc = {
        .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f))
                 * glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)),
        .descriptorHandles = {
                              static_cast<uint32_t>(*_envMappingHandle), static_cast<uint32_t>(*_lightHandle)}
      };

      vkCmdPushConstants(commandBuffer, _phongEnvMappingPipeline->getVkPipelineLayout(),
                         _phongEnvMappingPipeline->getPushConstantVkShaderStageFlags(), 0,
                         sizeof(envMapPc), &envMapPc);

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
};

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::GraphicsContext(
    std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice>&& physicalDevice,
    std::unique_ptr<LogicalDevice>&& logicalDevice, const FileLoader& fileLoader)
  : _instance(std::move(instance)), _debugMessenger(std::move(debugMessenger)),
    _physicalDevice(std::move(physicalDevice)), _logicalDevice(std::move(logicalDevice)),
    _fileLoader(fileLoader),
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

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
std::unique_ptr<common::GraphicsContext> GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::
    create(std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
           std::unique_ptr<PhysicalDevice>&& physicalDevice,
           std::unique_ptr<LogicalDevice>&& logicalDevice, const FileLoader& fileLoader) {
  return std::unique_ptr<GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>>(
      new GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>(
          std::move(instance), std::move(debugMessenger), std::move(physicalDevice),
          std::move(logicalDevice), fileLoader));
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::~GraphicsContext() {
  const VkDevice device = _logicalDevice->getVkDevice();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if constexpr (!SYNCED_OUTSIDE) {
      vkDestroySemaphore(device, _synchContext.renderFinishedSemaphores[i], nullptr);
      vkDestroySemaphore(device, _synchContext.imageAvailableSemaphores[i], nullptr);
    }
    vkDestroyFence(device, _frameFences[i], nullptr);
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
common::UpdateContextResponse GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::update(
    const common::UpdateContext& updateContext) {
  return {};
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::draw(
    const common::DrawingContext& drawingContext) {
  vkWaitForFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame],
                  VK_TRUE, UINT64_MAX);

  updateUniformBuffer(drawingContext.cameraContexts, _synchContext.currentFrame);

  _primaryCommandBuffer[_synchContext.currentFrame].resetCommandBuffer();
  for (int i = 0; i < MAX_THREADS_IN_POOL; i++) {
    _secondaryCommandBuffers[i][_synchContext.currentFrame].resetCommandBuffer();
  }

  vkResetFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame]);

  const common::CameraContext& cameraContext = drawingContext.cameraContexts[0];
  recordCommandBuffer(cameraContext.proj, cameraContext.view, drawingContext.imageIndex, drawingContext.screenSpaceViewPos);

  VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};

  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSemaphore waitSemaphore;
  if constexpr (!SYNCED_OUTSIDE) {
    waitSemaphore = _synchContext.imageAvailableSemaphores[_synchContext.currentFrame];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &waitSemaphore;
  }
  submitInfo.pWaitDstStageMask = waitStages;

  VkCommandBuffer submitCommands[] = {
    _primaryCommandBuffer[_synchContext.currentFrame].getVkCommandBuffer()};
  submitInfo.commandBufferCount = static_cast<uint32_t>(std::size(submitCommands));
  submitInfo.pCommandBuffers = submitCommands;

  VkSemaphore signalSemaphore;
  if constexpr (!SYNCED_OUTSIDE) {
    signalSemaphore = _synchContext.renderFinishedSemaphores[drawingContext.imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &signalSemaphore;
  }

  if (vkQueueSubmit(_logicalDevice->getGraphicsVkQueue(), 1, &submitInfo,
                    _frameFences[_synchContext.currentFrame])
      != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  if (++_synchContext.currentFrame == MAX_FRAMES_IN_FLIGHT) {
    _synchContext.currentFrame = 0;
  }
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::initializeResources() {
  setup();
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::waitCompleteExecution() const {
  vkWaitForFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame],
                  VK_TRUE, UINT64_MAX);
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::createPresentingResources(
    const common::PresentResources& presentResources) {
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
    createFsrContents(commandBuffer, fsrTexture, *_logicalDevice);

    _computeDescriptorSetWriter.storeImageStorage(fsrTexture);
    _computeDescriptorSetWriter.writeDescriptorSet(_logicalDevice->getVkDevice(),
        _computeDescriptorSet.getVkDescriptorSet());

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

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
void GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::waitDeviceIdle() const {
  vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

template <bool SYNCED_OUTSIDE, bool MULTIVIEW_PRESENTATION>
std::any
GraphicsContext<SYNCED_OUTSIDE, MULTIVIEW_PRESENTATION>::getSynchronizationContext() const {
  return std::make_any<const SynchronizationContext*>(&_synchContext);
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
          .buildImage(logicalDevice, commandBuffer, imageData.stagingBuffer.getVkBuffer(),
                      imageData.copyRegions, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice, commandBuffer, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
                            imageData.copyRegions, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
          .buildImage(logicalDevice, commandBuffer);
  texture.addCreateVkImageView(0, 1, 0, numLayers);
  return texture;
}

void createFsrContents(
    VkCommandBuffer commandBuffer, Texture& texture, const LogicalDevice& logicalDevice) {
  const VkExtent2D extent = texture.getVkExtent2D();
  lib::Buffer<std::byte> buffer(static_cast<size_t>(extent.width * extent.height), std::byte{0});
  Buffer stagingBuffer =
      Buffer::createStagingBuffer(logicalDevice, buffer.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
  stagingBuffer.copyData(std::span(static_cast<const std::byte*>(buffer.data()), buffer.size()));
  {
    VkBufferImageCopy imageCopy[] = {
      {
       .imageSubresource =
            {
              .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
              .mipLevel = 0,
              .baseArrayLayer = 0,
              .layerCount = 1,
            }, .imageExtent = {extent.width, extent.height, 1},
       }
    };
    texture.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    texture.copyFromStagingBuffer(commandBuffer, stagingBuffer.getVkBuffer(), imageCopy);
    texture.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_GENERAL);
  }
}

}  // namespace

}  // namespace vlkn
