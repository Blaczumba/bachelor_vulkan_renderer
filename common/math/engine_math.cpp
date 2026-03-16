#include "common/math/engine_math.h"

#include <glm/glm.hpp>

namespace common {

glm::vec3 getWorldSpaceViewDirection(
    uint16_t x, uint16_t y, uint16_t screenWidth, uint16_t screenHeight,
    const glm::mat4& invViewMatrix, const glm::mat4& invProjectionMatrix) {
  const glm::vec4 rayClip =
      glm::vec4((2.0f * x) / screenWidth - 1.0f, 1.0f - (2.0f * y) / screenHeight, -1.0f, 1.0f);
  const glm::vec4 rayEye = invProjectionMatrix * rayClip;
  return glm::normalize(glm::vec3(invViewMatrix * glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f)));
}

}  // namespace common
