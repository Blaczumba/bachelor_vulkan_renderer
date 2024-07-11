#include "fps_callback_manager.h"

FPSCallbackManager::FPSCallbackManager(std::shared_ptr<Window> window)
	: CallbackManager(window) {
	GLFWwindow* glfwWindow = _window->getGlfwWindow();
	glfwSetWindowUserPointer(glfwWindow, this);

	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);
	glfwSetKeyCallback(glfwWindow, keyCallback);
	glfwSetCursorPosCallback(glfwWindow, mouseCallback);
}

void FPSCallbackManager::pollEvents() {
	glfwPollEvents();
	processKeyboard();
}

void FPSCallbackManager::processKeyboard() {
	GLFWwindow* window = _window->getGlfwWindow();

	for (auto observer : _observers) {
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			observer->updateKeyboard({ 1.0f / 60.0f, GLFW_KEY_W });
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			observer->updateKeyboard({ 1.0f / 60.0f, GLFW_KEY_S });
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			observer->updateKeyboard({ 1.0f / 60.0f, GLFW_KEY_A });
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			observer->updateKeyboard({ 1.0f / 60.0f, GLFW_KEY_D });
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			observer->updateKeyboard({ 1.0f / 60.0f, GLFW_KEY_P });
	}
}

void FPSCallbackManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto camera = reinterpret_cast<FPSCamera*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void FPSCallbackManager::mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	auto cbManager = reinterpret_cast<FPSCallbackManager*>(glfwGetWindowUserPointer(window));

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	for (auto observer : cbManager->_observers) {
		observer->updateMouse({ 1.0f / 60.0f, xoffset, yoffset });
	}
}

void FPSCallbackManager::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	// auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}

bool FPSCallbackManager::firstMouse = true;
float FPSCallbackManager::lastX = 0.0f;
float FPSCallbackManager::lastY = 0.0f;