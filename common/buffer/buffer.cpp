#include "common/buffer/buffer.h"

#include <span>
#include <format>

#include "common/util/engine_exception.h"

namespace common {

void copyAndShrinkIndexData(std::span<std::byte> dst, std::span<const std::byte> src, size_t dstIndexSize,
                               size_t srcIndexSize, size_t offset) {
  if (dst.size() < dstIndexSize * src.size() / srcIndexSize + offset) [[unlikely]] {
    throw EngineException(std::format(
        "Trying to access out of range memory. Offset: {}, copied size: {}, buffer size: {}.",
        offset, dstIndexSize * src.size() / srcIndexSize, dst.size()));
  }

  std::byte* dstData = dst.data() + offset;
  const std::byte* srcData = src.data();
  for (size_t i = 0; i < src.size() / srcIndexSize; i++) {
    std::memcpy(dstData, srcData, dstIndexSize);
    dstData += dstIndexSize;
    srcData += srcIndexSize;
  }
}

}  // namespace common
