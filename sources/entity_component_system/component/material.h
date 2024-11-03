#pragma once

#include "component.h"

#include "memory_objects/uniform_buffer/uniform_buffer.h"

#include <memory>

class MaterialComponent : public Component {
	static constexpr ComponentType componentID = 3;
public:

	std::shared_ptr<UniformBufferTexture> diffuseMap;
	std::shared_ptr<UniformBufferTexture> normalMap;
	std::shared_ptr<UniformBufferTexture> metallicRoughnessMap;

	static constexpr std::enable_if_t < componentID < MAX_COMPONENTS, ComponentType> getComponentID() { return componentID; }
};
