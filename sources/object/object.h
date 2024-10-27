#pragma once

#include "descriptor_set/descriptor_set.h"
#include "entity_component_system/entity/entity.h"
#include "memory_objects/index_buffer.h"
#include "memory_objects/vertex_buffer.h"
#include "primitives/geometry.h"

#include <algorithm>
#include <memory>
#include <string_view>

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

class GameObject {
    Entity _entity;
    std::string_view _name;
    GameObject* _parent;
    std::vector<GameObject*> _children;

public:
    GameObject(const std::string_view name, Entity entity);
    void SetParent(GameObject* newParent);
};
