#include "surface.h"

#include <stdexcept>

Surface::Surface(std::shared_ptr<Instance> instance, std::shared_ptr<Window> window)
	: _instance(instance), _window(window) {
	if (glfwCreateWindowSurface(instance->getVkInstance(), window->getGlfwWindow(), nullptr, &_surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

Surface::~Surface() {
	vkDestroySurfaceKHR(_instance->getVkInstance(), _surface, nullptr);
}

VkSurfaceKHR Surface::getVkSurface() {
	return _surface;
}