#include "surface.h"

#include "instance/instance.h"
#include "window/window/window.h"

Surface::Surface(const std::shared_ptr<Instance>& instance, const std::shared_ptr<Window>& window)
	: _instance(instance), _window(window) {
	_surface = _window->createSurface(instance->getVkInstance());
}

Surface::~Surface() {
	vkDestroySurfaceKHR(_instance->getVkInstance(), _surface, nullptr);
}

const VkSurfaceKHR Surface::getVkSurface() const{
	return _surface;
}