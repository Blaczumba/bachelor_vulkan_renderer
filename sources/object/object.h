#pragma once

#include "memory_objects/vertex_buffer.h"
#include "memory_objects/index_buffer.h"
#include "descriptor_set/descriptor_set.h"
#include "primitives/geometry.h"

#include <memory>

#include <glm/glm.hpp>

struct Object {
    std::unique_ptr<VertexBuffer> vertexBufferPTNTB;
    std::unique_ptr<VertexBuffer> vertexBufferP;
    std::unique_ptr<IndexBuffer> indexBuffer;

    uint32_t dynamicUniformIndex;

    std::unique_ptr<DescriptorSet> _descriptorSet;

    glm::mat4 model;
    AABB volume;
};