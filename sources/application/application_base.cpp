#include "application_base.h"

#include "window/window/window_glfw.h"

ApplicationBase::ApplicationBase() {
	// glfwInit();
	// glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window = std::make_shared<WindowGLFW>("Bejzak Engine", 1920, 1080);
	_instance = std::make_shared<Instance>("Bejzak Engine");

#ifdef VALIDATION_LAYERS_ENABLED
	_debugMessenger = std::make_shared<DebugMessenger>(_instance);
#endif // VALIDATION_LAYERS_ENABLED

	_surface = std::make_shared<Surface>(_instance, _window);
	_physicalDevice = std::make_unique<PhysicalDevice>(_instance, _surface);
	_logicalDevice = _physicalDevice->createLogicalDevice();
	_swapchain = std::make_unique<Swapchain>(*_surface, *_window, *_logicalDevice);
}

ApplicationBase::~ApplicationBase() {
	// glfwTerminate();
}
