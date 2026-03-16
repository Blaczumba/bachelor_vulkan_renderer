#pragma once

#include <span>

namespace common {

struct AttributeDescription {
  void* data;
  size_t size;
  size_t count;
};

void copyAndShrinkIndexData(std::span<std::byte> dst, std::span<const std::byte> src,
                            size_t dstIndexSize, size_t srcIndexSize, size_t offset = 0);

void copyDataInterleaving(std::span<std::byte> dst, std::span<const AttributeDescription> attributes);

}  // namespace common
