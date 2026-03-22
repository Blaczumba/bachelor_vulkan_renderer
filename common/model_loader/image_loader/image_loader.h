#pragma once

#include <ktx.h>
#include <lib/buffer/buffer.h>
#include <span>
#include <stb_image/stb_image.h>
#include <string_view>
#include <tuple>
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

struct StbiDeleter {
  void operator()(stbi_uc* p) const {
    stbi_image_free(p);
  }
};

struct KtxDeleter {
  void operator()(ktxTexture* p) const {
    ktxTexture_Destroy(p);
  }
};

using StbUniquePtr = std::unique_ptr<stbi_uc, StbiDeleter>;
using KtxUniquePtr = std::unique_ptr<ktxTexture, KtxDeleter>;
using OwnedImageResources = std::variant<StbUniquePtr, KtxUniquePtr>;

struct ImageResource {
  uint32_t width;
  uint32_t height;
  uint32_t mipLevels;
  uint32_t layerCount;
  lib::Buffer<ImageSubresource> subresources;
  const void* data;
  size_t size;
};

std::tuple<ImageResource, OwnedImageResources> loadImageStbi(std::span<const std::byte> imageData);

std::tuple<ImageResource, OwnedImageResources> loadImageKtx(std::span<const std::byte> imageData);

std::tuple<ImageResource, OwnedImageResources> loadImage(
    std::span<const std::byte> imageData, std::string_view filePath);
