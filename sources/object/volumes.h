#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <vector>
#include <limits>

struct AABB {
	glm::vec3 lowerCorner;
	glm::vec3 upperCorner;

	bool contains(const AABB& other) const;
	void extend(const AABB& other);
};

AABB createAABBfromVertices(const std::vector<glm::vec3>& vertices, const glm::mat4& transform = glm::mat4(1.0f));