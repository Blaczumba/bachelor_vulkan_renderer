#pragma once

#include <span>

namespace common {

void copyAndShrinkIndexData(std::span<std::byte> dst, std::span<const std::byte> src,
                            size_t dstIndexSize, size_t srcIndexSize, size_t offset = 0);

}  // namespace common
