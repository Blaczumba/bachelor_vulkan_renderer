#pragma once

#include "entity_component_system/entity/entity.h"
#include "memory_objects/vertex_buffer.h"
#include "memory_objects/index_buffer.h"
#include "primitives/geometry.h"

#include <memory>

class MeshComponent {
	static constexpr ComponentType componentID = 2;

public:
	std::shared_ptr<VertexBuffer> vertexBuffer;
	std::shared_ptr<IndexBuffer> indexBuffer;
	std::shared_ptr<VertexBuffer> vertexBufferPrimitive;
	AABB aabb;

	static constexpr std::enable_if_t<componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};
