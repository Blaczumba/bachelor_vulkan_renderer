#include "application_base.h"

#include <GLFW/glfw3.h>

#include "window/window/window_glfw.h"

ApplicationBase::ApplicationBase() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*>requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifdef VALIDATION_LAYERS_ENABLED
	requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // VALIDATION_LAYERS_ENABLED
	requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

	_instance = std::make_shared<Instance>("Bejzak Engine", requiredExtensions);
	_window = std::make_shared<WindowGLFW>(*_instance, "Bejzak Engine", 1920, 1080);

#ifdef VALIDATION_LAYERS_ENABLED
	_debugMessenger = std::make_shared<DebugMessenger>(_instance);
#endif // VALIDATION_LAYERS_ENABLED

	_physicalDevice = std::make_unique<PhysicalDevice>(*_window);
	_logicalDevice = _physicalDevice->createLogicalDevice();
	_swapchain = std::make_unique<Swapchain>(*_logicalDevice);

	_singleTimeCommandPool = std::make_unique<CommandPool>(*_logicalDevice);
}

ApplicationBase::~ApplicationBase() {
	glfwTerminate();
}
