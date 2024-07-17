#pragma once

#include <cstdint>

#include <glm/glm.hpp>

struct Extent {
	uint32_t width;
	uint32_t height;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec2 texCoord;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct UniformBufferObjectMP {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 proj;
};
