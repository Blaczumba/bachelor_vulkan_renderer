#pragma once

#include <cstdint>
#include <glm/glm.hpp>

struct Extent {
  uint32_t width;
  uint32_t height;
};

struct UniformBufferLight {
  alignas(16) glm::mat4 projView;
  alignas(16) glm::vec3 pos;
};

struct UniformBufferCamera {
  alignas(16) glm::mat4 view;
  alignas(16) glm::mat4 proj;
  alignas(16) glm::vec3 pos;
  alignas(16) glm::vec3 viewDir;
};
