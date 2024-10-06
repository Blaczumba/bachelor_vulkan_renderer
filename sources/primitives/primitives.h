#pragma once

#include <cstdint>

#include <glm/glm.hpp>

struct Extent {
	uint32_t width;
	uint32_t height;
};

//struct VertexP {
//    glm::vec3 pos;
//};

using VertexP = glm::vec3;

struct VertexPT {
    glm::vec3 pos;
    glm::vec2 texCoord;

    static constexpr size_t num_attributes = 2;
};

struct VertexPTN {
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;

    static constexpr size_t num_attributes = 3;
};

struct VertexPTNT {
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;

    static constexpr size_t num_attributes = 4;
};

struct VertexPTNTB {
    glm::vec3 pos;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    static constexpr size_t num_attributes = 5;
};

struct UniformBufferLight {
    alignas(16) glm::mat4 projView;
    alignas(16) glm::vec3 pos;
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;

};

struct UniformBufferCamera {
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 pos;
};

