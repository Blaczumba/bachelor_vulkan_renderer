#pragma once

#include <any>
#include <array>
#include <iostream>
#include <vector>

#include "common/abstractions/graphics_context.h"
#include "common/entity_component_system/registry/registry.h"
#include "common/model_loader/model_loader.h"
#include "common/object/object.h"
#include "common/scene/octree.h"
#include "common/util/primitives.h"
#include "vulkan/graphics_context/graphics_context.h"
#include "vulkan/resource_manager/asset_manager.h"
#include "vulkan/resource_manager/bindless_descriptor_set_writer.h"
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

template <bool SYNCED_OUTSIDE>
class GraphicsContext final : public common::GraphicsContext {
  GraphicsContext(std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
                  std::unique_ptr<PhysicalDevice>&& physicalDevice,
                  std::unique_ptr<LogicalDevice>&& logicalDevice,
                  const SwapchainContext& swapchainContext, const FileLoader& fileLoader);

public:
  static std::unique_ptr<common::GraphicsContext> create(
      std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
      std::unique_ptr<PhysicalDevice>&& physicalDevice,
      std::unique_ptr<LogicalDevice>&& logicalDevice, const SwapchainContext& swapchainContext,
      const FileLoader& fileLoader);

  ~GraphicsContext();

  common::UpdateContextResponse update(const common::UpdateContext& updateContext) override;

  void draw(const common::DrawingContext& drawingContext) override;

  void waitCompleteExecution() const override;

  void waitDeviceIdle() const override;

  std::any getSynchronizationContext() const override;

  static constexpr uint32_t MAX_THREADS_IN_POOL = 2;

private:
  std::unique_ptr<Instance> _instance;
  DebugMessenger _debugMessenger;
  std::unique_ptr<PhysicalDevice> _physicalDevice;
  std::unique_ptr<LogicalDevice> _logicalDevice;

  const FileLoader& _fileLoader;

  SwapchainContext _swapchainContexts;
  SynchronizationContext _synchContext;

  std::array<VkFence, MAX_FRAMES_IN_FLIGHT> _frameFences;

  std::shared_ptr<CommandPool> _singleTimeCommandPool;

  std::unique_ptr<AssetManager> _assetManager;
  std::unique_ptr<GpuBufferManager> _gpuBufferManager;
  std::unique_ptr<SamplerManager> _samplerManager;
  std::unique_ptr<PipelineManager> _pipelineManager;

  std::shared_ptr<DescriptorPool> _bindlessDescriptorPool;
  DescriptorSet _bindlessDescriptorSet;
  std::unique_ptr<BindlessDescriptorSetWriter> _bindlessWriter;

  std::shared_ptr<DescriptorPool> _dynamicDescriptorPool;
  DescriptorSet _dynamicDescriptorSet;
  DescriptorSetWriter _dynamicDescriptorSetWriter;

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
  std::vector<Texture> _attachments;

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

  UniformBufferCamera _ubCamera;
  UniformBufferLight _ubLight;
  Buffer _dynamicUniformBuffersCamera;
  Buffer _lightBuffer;
  UniformBufferHandle _lightHandle;

  void setup();
  void loadCubemap(const VertexData& cubeData);
  void createDescriptorSets();
  void createPresentResources(const SwapchainContext& context);
  void createEnvMappingResources();
  void createShadowResources();
  void createGraphicsPipelines();
  void createCommandBuffers();
  void createSyncObjects();
  std::tuple<UniformTextureHandle, GpuTextureHandle> getOrLoadTexture(
      std::unordered_map<StagingImageDataResourceHandle,
                         std::pair<UniformTextureHandle, GpuTextureHandle>>& textureCache,
      StagingImageDataResourceHandle textureID, VkFormat format, VkCommandBuffer commandBuffer,
      float maxSamplerAnisotropy, SamplerHandle samplerHandle);
  void loadObjects(std::span<const VertexData> sceneData);
  void createOctreeScene();
  void recordShadowCommandBuffer(VkCommandBuffer commandBuffer);
  void recordEnvMappingCommandBuffer(VkCommandBuffer commandBuffer);
  void updateUniformBuffer(Camera camera, uint32_t currentFrame);
  void recordOctreeSecondaryCommandBuffer(
      const VkCommandBuffer commandBuffer, const OctreeNode* rootNode,
      std::span<const glm::vec4> planes);
  void recordCommandBuffer(
      const glm::mat4& cameraProj, const glm::mat4& cameraView, uint32_t imageIndex);
};

}  // namespace vlkn

namespace vlkn {

template <bool SYNCED_OUTSIDE>
GraphicsContext<SYNCED_OUTSIDE>::GraphicsContext(
    std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice>&& physicalDevice,
    std::unique_ptr<LogicalDevice>&& logicalDevice, const SwapchainContext& swapchainContext,
    const FileLoader& fileLoader)
  : _instance(std::move(instance)), _debugMessenger(std::move(debugMessenger)),
    _physicalDevice(std::move(physicalDevice)), _logicalDevice(std::move(logicalDevice)),
    _fileLoader(fileLoader),
    _singleTimeCommandPool(
        CommandPool::create(*_logicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)),
    _assetManager(AssetManager::create(*_logicalDevice, fileLoader)),
    _gpuBufferManager(GpuBufferManager::create()), _samplerManager(SamplerManager::create()),
    _pipelineManager(PipelineManager::create(fileLoader)),
    _bindlessDescriptorPool(DescriptorPool::create(
        *_logicalDevice, 1, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)),
    _bindlessDescriptorSet(_bindlessDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateBindlessLayout(*_logicalDevice))),
    _bindlessWriter(BindlessDescriptorSetWriter::create(_bindlessDescriptorSet)),
    _dynamicDescriptorPool(DescriptorPool::create(*_logicalDevice, 1)),
    _dynamicDescriptorSet(_dynamicDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateCameraLayout(*_logicalDevice))) {
  createPresentResources(swapchainContext);
  setup();
}

template <bool SYNCED_OUTSIDE>
std::unique_ptr<common::GraphicsContext> GraphicsContext<SYNCED_OUTSIDE>::create(
    std::unique_ptr<Instance>&& instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice>&& physicalDevice,
    std::unique_ptr<LogicalDevice>&& logicalDevice, const SwapchainContext& swapchainContext,
    const FileLoader& fileLoader) {
  return std::unique_ptr<GraphicsContext<SYNCED_OUTSIDE>>(new GraphicsContext<SYNCED_OUTSIDE>(
      std::move(instance), std::move(debugMessenger), std::move(physicalDevice),
      std::move(logicalDevice), swapchainContext, fileLoader));
}

template <bool SYNCED_OUTSIDE>
GraphicsContext<SYNCED_OUTSIDE>::~GraphicsContext() {
  const VkDevice device = _logicalDevice->getVkDevice();

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(device, _synchContext.renderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(device, _synchContext.imageAvailableSemaphores[i], nullptr);
    vkDestroyFence(device, _frameFences[i], nullptr);
  }
}

template <bool SYNCED_OUTSIDE>
common::UpdateContextResponse GraphicsContext<SYNCED_OUTSIDE>::update(
    const common::UpdateContext& updateContext) {
  return {};
}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::draw(const common::DrawingContext& drawingContext) {
  vkWaitForFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame],
                  VK_TRUE, UINT64_MAX);

  updateUniformBuffer(drawingContext.camera, _synchContext.currentFrame);

  vkResetFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame]);

  const glm::mat4 cameraView = drawingContext.camera.getViewMatrix();
  const glm::mat4 cameraProj = drawingContext.camera.getProjectionMatrix();
  recordCommandBuffer(cameraProj, cameraView, drawingContext.imageIndex);

  VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};

  VkSemaphore waitSemaphores[] = {
    _synchContext.imageAvailableSemaphores[_synchContext.currentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  VkCommandBuffer submitCommands[] = {
    _primaryCommandBuffer[_synchContext.currentFrame].getVkCommandBuffer()};
  submitInfo.commandBufferCount = static_cast<uint32_t>(std::size(submitCommands));
  submitInfo.pCommandBuffers = submitCommands;

  VkSemaphore signalSemaphores[] = {
    _synchContext.renderFinishedSemaphores[drawingContext.imageIndex]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(_logicalDevice->getGraphicsVkQueue(), 1, &submitInfo,
                    _frameFences[_synchContext.currentFrame])
      != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  if (++_synchContext.currentFrame == MAX_FRAMES_IN_FLIGHT) {
    _synchContext.currentFrame = 0;
  }
}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::waitCompleteExecution() const {
  vkWaitForFences(_logicalDevice->getVkDevice(), 1, &_frameFences[_synchContext.currentFrame],
                  VK_TRUE, UINT64_MAX);
}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::waitDeviceIdle() const {
  vkDeviceWaitIdle(_logicalDevice->getVkDevice());
}

template <bool SYNCED_OUTSIDE>
std::any GraphicsContext<SYNCED_OUTSIDE>::getSynchronizationContext() const {
  return std::make_any<const SynchronizationContext*>(&_synchContext);
}

}  // namespace vlkn

#include "vulkan/graphics_context/graphics_context.ipp"
