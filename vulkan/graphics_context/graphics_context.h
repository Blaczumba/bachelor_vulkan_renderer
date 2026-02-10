#pragma once

#include <any>
#include <array>
#include <iostream>
#include <vector>

#include "common/abstractions/graphics_context.h"
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
#include "common/object/object.h"
#include "common/scene/octree.h"
#include "common/entity_component_system/registry/registry.h"
#include "common/util/primitives.h"
#include "common/model_loader/model_loader.h"

/// 
#include "vulkan/wrapper/render_pass/render_pass.h"
#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/texture.h"
#include "vulkan/wrapper/descriptor_set/descriptor_set_layout.h"
#include "vulkan/wrapper/framebuffer/framebuffer.h"


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
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
  std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> _imageAvailableSemaphores;
  std::vector<VkSemaphore> _renderFinishedSemaphores;
};

template <bool SYNCED_OUTSIDE>
class GraphicsContext final : public common::GraphicsContext {
  GraphicsContext(Instance&& instance, DebugMessenger&& debugMessenger,
                  std::unique_ptr<PhysicalDevice>&& physicalDevice, LogicalDevice&& logicalDevice,
                  const FileLoader& fileLoader);

public:
  static std::unique_ptr<common::GraphicsContext> create(
      Instance&& instance, DebugMessenger&& debugMessenger,
      std::unique_ptr<PhysicalDevice>&& physicalDevice, LogicalDevice&& logicalDevice,
      const FileLoader& fileLoader);

  common::UpdateContextResponse update(const common::UpdateContext& updateContext) override;

  void draw(const common::DrawingContext& drawingContext) override;

  void waitCompleteExecution() const override;

  std::any getSynchronizationContext() const override;

  static constexpr uint32_t MAX_THREADS_IN_POOL = 2;

private:
  Instance _instance;
  DebugMessenger _debugMessenger;
  std::unique_ptr<PhysicalDevice> _physicalDevice;
  LogicalDevice _logicalDevice;

  const FileLoader& _fileLoader;

  SynchronizationContext _synchContext;

  std::array<VkFence, MAX_FRAMES_IN_FLIGHT> _frameFences;

  CommandPool _singleTimeCommandPool;

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

  void setup();
  void loadCubemap(const VertexData& cubeData);
};

}  // namespace vlkn
