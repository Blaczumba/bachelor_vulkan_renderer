#include "component.h"

#include "descriptor_set/descriptor_set.h"
#include "memory_objects/texture/texture.h"

#include <memory>

class MaterialComponent : public Component {
	static constexpr ComponentType componentID = 3;

	DescriptorSet* _descriptorSet;
	uint32_t _dynamicUniformIndex;

public:
	static constexpr std::enable_if_t < componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};
