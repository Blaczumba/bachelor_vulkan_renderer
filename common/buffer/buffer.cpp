#include "common/buffer/buffer.h"

#include <algorithm>
#include <span>
#include <ranges>
#include <format>
#include <vector>

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

void copyDataInterleaving(
    std::span<std::byte> dst, std::span<const AttributeDescription> attributes) {
  if (attributes.empty()) {
    throw EngineException("AttributeDescriptions cannot be empty.");
  }

  const size_t count = attributes[0].count;

  if (std::any_of(std::cbegin(attributes), std::cend(attributes),
                  [count](const AttributeDescription& attribute) {
                    return attribute.count != count;
                  })) {
    throw EngineException(
        "Buffers must have equal number of elements when copying buffers in an interleaving "
        "manner.");
  }

  std::vector<std::byte*> offsetMemory;
  offsetMemory.reserve(attributes.size());
  offsetMemory.push_back(dst.data());
  size_t stride = 0;
  std::transform(std::cbegin(attributes), std::prev(std::cend(attributes)),
                 std::back_inserter(offsetMemory), [&](const AttributeDescription& attribute) {
                   stride += attribute.size;
                   return dst.data() + stride;
                 });
  stride += attributes.back().size;

  for (size_t j = 0, running_stride = 0; j < count; j++, running_stride += stride) {
    for (const auto& [offset, attribute] : std::views::zip(offsetMemory, attributes)) {
      std::memcpy(offset + running_stride,
                  static_cast<uint8_t*>(attribute.data) + j * attribute.size, attribute.size);
    }
  }
}

}  // namespace common
