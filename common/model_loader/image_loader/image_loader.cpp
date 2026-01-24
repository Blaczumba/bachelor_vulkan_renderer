#include "image_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <cmath>
#include <format>
#include <span>
#include <stb_image/stb_image.h>
#include <vector>

#include "common/util/engine_exception.h"

ImageResource loadImageStbi(std::span<const std::byte> imageData) {
  int width, height, channels;
  stbi_uc* pixels = stbi_load_from_memory(
      reinterpret_cast<const stbi_uc*>(imageData.data()), static_cast<int>(imageData.size()),
      &width, &height, &channels, STBI_rgb_alpha);
  if (!pixels) [[unlikely]] {
    throw EngineException("Failed to load image file (stbi).");
  }

  return ImageResource{
    .libraryResource = pixels,
    .width = static_cast<uint32_t>(width),
    .height = static_cast<uint32_t>(height),
    .mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1,
    .layerCount = 1,
    .subresources = {ImageSubresource{
      .layerCount = 1,
      .width = static_cast<uint32_t>(width),
      .height = static_cast<uint32_t>(height),
      .depth = 1,
    }},
    .data = pixels,
    .size = static_cast<uint32_t>(4 * width * height)};
}

ImageResource loadImageKtx(std::span<const std::byte> imageData) {
  ktxTexture* ktxTexture;
  if (ktxResult result = ktxTexture_CreateFromMemory(
          reinterpret_cast<const ktx_uint8_t*>(imageData.data()), imageData.size(),
          KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
      result != KTX_SUCCESS) [[unlikely]] {
    throw EngineException("Failed to load image file (ktx).");
  }

  ImageResource image{
    .libraryResource = ktxTexture,
    .width = ktxTexture->baseWidth,
    .height = ktxTexture->baseHeight,
    .mipLevels = ktxTexture->numLevels,
    .layerCount = ktxTexture->numFaces,
    .subresources = lib::Buffer<ImageSubresource>(ktxTexture->numLevels*ktxTexture->numFaces),
    .data = ktxTexture->pData,
    .size = ktxTexture->dataSize};

  for (uint32_t face = 0, index = 0; face < image.layerCount; face++) {
    for (uint32_t level = 0; level < image.mipLevels; level++, index++) {
      ktx_size_t offset;
      if (ktxResult result = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
          result != KTX_SUCCESS) [[unlikely]] {
        ktxTexture_Destroy(ktxTexture);
        throw EngineException(
            std::format("Failed to get image offset for level: {}, face: {} (ktx).", level, face));
      }

      image.subresources[index] = ImageSubresource{
        .offset = offset,
        .mipLevel = level,
        .baseArrayLayer = face,
        .layerCount = 1,
        .width = image.width >> level,
        .height = image.height >> level,
        .depth = 1,
      };
    }
  }
  return image;
}

ImageResource loadImage(std::span<const std::byte> imageData, std::string_view filePath) {
  if (filePath.ends_with(".ktx") || filePath.ends_with(".ktx2")) {
    return loadImageKtx(imageData);
  } else {
    return loadImageStbi(imageData);
  }
}

namespace {

struct Deallocator {
  void operator()(ktxTexture* texture) {
    ktxTexture_Destroy(texture);
  }

  void operator()(stbi_uc* texture) {
    stbi_image_free(texture);
  }

  void operator()(auto&&) {}
};

}  // namespace

void deallocateResources(ImageResource& resource) {
  std::visit(Deallocator{}, resource.libraryResource);
}
