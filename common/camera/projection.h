#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <variant>

struct OrthographicProjection {
  float left, right, bottom, top, nearZ, farZ;
};

struct PerspectiveProjection {
  float fovy, aspect, nearZ, farZ;
};

using Projection = std::variant<OrthographicProjection, PerspectiveProjection>;

struct CalculateProjectionVisitor {
  glm::mat4 operator()(const OrthographicProjection& ortho) {
    glm::mat4 matrix = glm::ortho(ortho.left, ortho.right, ortho.bottom, ortho.top, ortho.nearZ, ortho.farZ);
    matrix[1][1] = -matrix[1][1];  // Flip Y for Vulkan-style clip space.
    return matrix;
  }

  glm::mat4 operator()(const PerspectiveProjection& persp) {
    glm::mat4 matrix = glm::perspective(persp.fovy, persp.aspect, persp.nearZ, persp.farZ);
    matrix[1][1] = -matrix[1][1];  // Flip Y for Vulkan-style clip space.
    return matrix;
  }
};
