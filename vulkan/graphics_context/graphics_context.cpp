#include "vulkan/graphics_context/graphics_context.h"

#include "common/model_loader/tiny_gltf_loader/tiny_gltf_loader.h"

#include <any>

namespace vlkn {

template <bool SYNCED_OUTSIDE>
GraphicsContext<SYNCED_OUTSIDE>::GraphicsContext(
    Instance&& instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice>&& physicalDevice, LogicalDevice&& logicalDevice,
    const FileLoader& fileLoader)
  : _instance(std::move(instance)), _debugMessenger(std::move(debugMessenger)),
    _physicalDevice(std::move(physicalDevice)), _logicalDevice(std::move(logicalDevice)),
    _fileLoader(fileLoader),
    _singleTimeCommandPool(
        CommandPool::create(_logicalDevice, VK_COMMAND_POOL_CREATE_TRANSIENT_BIT)),
    _assetManager(AssetManager::create(_logicalDevice, fileLoader)),
    _gpuBufferManager(GpuBufferManager::create()), _samplerManager(SamplerManager::create()),
    _pipelineManager(PipelineManager::create(fileLoader)),
    _bindlessDescriptorPool(
        DescriptorPool::create(_logicalDevice, 1, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)),
    _bindlessDescriptorSet(_bindlessDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateBindlessLayout(_logicalDevice))),
    _dynamicDescriptorPool(DescriptorPool::create(_logicalDevice, 1)),
    _dynamicDescriptorSet(_dynamicDescriptorPool->createDesriptorSet(
        _pipelineManager->getOrCreateCameraLayout(_logicalDevice))) {}

template <bool SYNCED_OUTSIDE>
std::unique_ptr<common::GraphicsContext> GraphicsContext<SYNCED_OUTSIDE>::create(
    Instance&& instance, DebugMessenger&& debugMessenger,
    std::unique_ptr<PhysicalDevice>&& physicalDevice, LogicalDevice&& logicalDevice,
    const FileLoader& fileLoader) {
  return std::unique_ptr<GraphicsContext<SYNCED_OUTSIDE>>(
      new GraphicsContext<SYNCED_OUTSIDE>(
          std::move(instance), std::move(physicalDevice), std::move(logicalDevice)),
      fileLoader);
}

template <bool SYNCED_OUTSIDE>
common::UpdateContextResponse GraphicsContext<SYNCED_OUTSIDE>::update(
    const common::UpdateContext& updateContext) {
  return {};
}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::draw(const common::DrawingContext& drawingContext) {}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::waitCompleteExecution() const {
  vkWaitForFences(_logicalDevice.getVkDevice(), 1, &_frameFences[_synchContext.currentFrame],
                  VK_TRUE, UINT64_MAX);
}

template <bool SYNCED_OUTSIDE>
std::any GraphicsContext<SYNCED_OUTSIDE>::getSynchronizationContext() const {
  return std::make_any<const SynchronizationContext*>(&_synchContext);
}


// BELOW CODE IS FOR DEMONSTRATION:

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

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::setup() {
  std::string data = _fileLoader->loadFileToString(MODELS_PATH "cone.obj");
  VertexData cubeData = loadObj(*_assetManager, "cube.obj", data);
  const std::vector<VertexData> sceneData =
      LoadGltfFromFile(*_assetManager, MODELS_PATH "sponza/scene.gltf");
  cubeData.diffuseTexture = {
    _assetManager->loadImageAsync(TEXTURES_PATH "cubemap_yokohama_rgba.ktx"),
    TEXTURES_PATH "cubemap_yokohama_rgba.ktx"};
}

template <bool SYNCED_OUTSIDE>
void GraphicsContext<SYNCED_OUTSIDE>::loadCubemap(const VertexData& cubeData) {
  SingleTimeCommandBuffer handle(*_singleTimeCommandPool);
  const VkCommandBuffer commandBuffer = handle.getCommandBuffer();

  const AssetManager::ImageData& imageData =
      _assetManager->getImageData(cubeData.diffuseTexture.ID);

  _textureCubemap = createSkybox(_logicalDevice, commandBuffer, imageData, VK_FORMAT_R8G8B8A8_UNORM,
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

}  // namespace vlkn
