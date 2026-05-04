#include "vulkan/wrapper/util/util.h"

bool hasStencil(VkFormat format) {
  static constexpr VkFormat formats[] = {VK_FORMAT_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                         VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT};
  return std::find(std::cbegin(formats), std::cend(formats), format) != std::cend(formats);
}
