#pragma once

#include <cstdint>

#include <glm/glm.hpp>

struct Extent {
	uint32_t width;
	uint32_t height;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};