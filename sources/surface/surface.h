#pragma once

#include "instance/instance.h"
#include "window/window.h"

#include <memory>

class Surface {
	VkSurfaceKHR _surface;
	std::shared_ptr<Window> _window;
	std::shared_ptr<Instance> _instance;
public:
	Surface(std::shared_ptr<Instance> instance, std::shared_ptr<Window> window);
	~Surface();
	VkSurfaceKHR getVkSurface();
};