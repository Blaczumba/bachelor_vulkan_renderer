#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "common/buffer/buffer.h"

struct BufferDescription {
  std::string name;
  std::vector<common::AttributeDescription> attributes;
  size_t totalSize;
};

std::vector<BufferDescription> analyzeConfig(
    std::span<const std::pair<std::string, std::string>> orders,
    std::span<const common::AttributeDescription> descs);

size_t getShrunkIndexSize(std::span<const std::byte> indicesBuffer, size_t indexSize);

void copyAndShrinkIndices(void* dstIndices, size_t dstIndexSize, const void* srcIndices,
                          size_t srcIndexSize, size_t count);
