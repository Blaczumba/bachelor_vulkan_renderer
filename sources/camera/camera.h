#pragma once

#include <glm/glm.hpp>

class Camera {
public:
	Camera();
	virtual glm::mat4 getViewMatrix() const =0;
};