#include "surface.h"

#include <stdexcept>

Surface::Surface(std::shared_ptr<Instance> instance, std::shared_ptr<Window> window)
	: _instance(instance), _window(window) {
	_surface = _window->createSurface(instance->getVkInstance());
}

Surface::~Surface() {
	vkDestroySurfaceKHR(_instance->getVkInstance(), _surface, nullptr);
}

VkSurfaceKHR Surface::getVkSurface() {
	return _surface;
}