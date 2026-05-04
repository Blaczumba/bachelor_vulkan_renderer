#pragma once

#include <format>
#include <vulkan/vulkan.h>

#include "common/util/engine_exception.h"

constexpr VkIndexType getIndexType(uint8_t indexSize) {
  switch (indexSize) {
    case 1:
      return VK_INDEX_TYPE_UINT8_EXT;
    case 2:
      return VK_INDEX_TYPE_UINT16;
    case 4:
      return VK_INDEX_TYPE_UINT32;
    default:
      throw EngineException(std::format("Unrecognized index size of {}.", indexSize));
  }
}

constexpr uint32_t getIndexSize(VkIndexType type) {
  switch (type) {
    case VK_INDEX_TYPE_UINT8_EXT:
      return 1;
    case VK_INDEX_TYPE_UINT16:
      return 2;
    case VK_INDEX_TYPE_UINT32:
      return 4;
    default:
      throw EngineException(
          std::format("Unrecognized VkIndexType of {}.", static_cast<uint32_t>(type)));
  }
}
