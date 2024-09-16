#include "volumes.h"

bool AABB::contains(const AABB& other) const {
	const glm::vec3 otherLowerCorner = other.lowerCorner;
	const glm::vec3 otherUpperCorner = other.upperCorner;
	return lowerCorner.x <= other.lowerCorner.x && lowerCorner.y <= otherLowerCorner.y && lowerCorner.z <= otherLowerCorner.z &&
		upperCorner.x >= otherUpperCorner.x && upperCorner.y >= otherUpperCorner.y && upperCorner.z >= otherUpperCorner.z;
}

void AABB::extend(const AABB& other) {
	lowerCorner.x = std::min(lowerCorner.x, other.lowerCorner.x);
	lowerCorner.y = std::min(lowerCorner.y, other.lowerCorner.y);
	lowerCorner.z = std::min(lowerCorner.z, other.lowerCorner.z);

	upperCorner.x = std::max(upperCorner.x, other.upperCorner.x);
	upperCorner.y = std::max(upperCorner.y, other.upperCorner.y);
	upperCorner.z = std::max(upperCorner.z, other.upperCorner.z);
}

AABB createAABBfromVertices(const std::vector<glm::vec3>& vertices, const glm::mat4& transform) {
	AABB volume = {
		.lowerCorner = { std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max() },
		.upperCorner = { std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min() }
	};

	for (glm::vec3 vertex : vertices) {
		vertex = transform * glm::vec4(vertex, 1.0f);
		volume.lowerCorner.x = std::min(volume.lowerCorner.x, vertex.x);
		volume.lowerCorner.y = std::min(volume.lowerCorner.y, vertex.y);
		volume.lowerCorner.z = std::min(volume.lowerCorner.z, vertex.z);

		volume.upperCorner.x = std::max(volume.upperCorner.x, vertex.x);
		volume.upperCorner.y = std::max(volume.upperCorner.y, vertex.y);
		volume.upperCorner.z = std::max(volume.upperCorner.z, vertex.z);
	}

	return volume;
}
