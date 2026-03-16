#pragma once

#include <glm/glm.hpp>

namespace common {

glm::vec3 getWorldSpaceViewDirection(
    uint16_t x, uint16_t y, uint16_t screenWidth, uint16_t screenHeight,
    const glm::mat4& invViewMatrix, const glm::mat4& invProjectionMatrix);

}  // namespace common
