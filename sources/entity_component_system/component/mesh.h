#include "component.h"
#include "memory_objects/vertex_buffer.h"
#include "memory_objects/index_buffer.h"

#include <memory>

class MeshComponent : public Component {
	static constexpr ComponentType componentID = 2;

	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;

public:
	static constexpr std::enable_if_t<componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};
