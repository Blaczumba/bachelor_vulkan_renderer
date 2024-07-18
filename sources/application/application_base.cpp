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
	_physicalDevice = std::make_shared<PhysicalDevice>(_instance, _surface);
	_logicalDevice = std::make_shared<LogicalDevice>(_physicalDevice);
	_swapchain = std::make_shared<Swapchain>(_surface, _window, _logicalDevice, _physicalDevice);
}

ApplicationBase::~ApplicationBase() {
	// glfwTerminate();
}
