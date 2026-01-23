#pragma once

#include <lib/buffer/buffer.h>

#include <ktx.h>
#include <span>
#include <stb_image/stb_image.h>
#include <string_view>
#include <variant>

struct ImageSubresource {
  size_t offset;
  uint32_t mipLevel;
  uint32_t baseArrayLayer;
  uint32_t layerCount;
  uint32_t width;
  uint32_t height;
  uint32_t depth;
};

struct ImageResource {
  std::variant<stbi_uc*, ktxTexture*> libraryResource;
  uint32_t width;
  uint32_t height;
  uint32_t mipLevels;
  uint32_t layerCount;
  lib::Buffer<ImageSubresource> subresources;
  void* data;
  size_t size;
};

ImageResource loadImageStbi(std::span<const std::byte> imageData);

ImageResource loadImageKtx(std::span<const std::byte> imageData);

ImageResource loadImage(std::span<const std::byte> imageData, std::string_view filePath);

void deallocateResources(ImageResource& resource);
