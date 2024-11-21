#include "fps_callback_manager.h"

#include "window/callback_observer/callback_observer.h"
#include "window/window/window_glfw.h"

#include <GLFW/glfw3.h>

#include <chrono>

FPSCallbackManager::FPSCallbackManager(WindowGLFW* window) : _window(window) {
	GLFWwindow* glfwWindow = _window->getGlfwWindow();
	glfwSetWindowUserPointer(glfwWindow, this);

	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	// glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);
	glfwSetKeyCallback(glfwWindow, keyCallback);
	glfwSetCursorPosCallback(glfwWindow, mouseCallback);
}

CallbackData FPSCallbackManager::_data = {};

void FPSCallbackManager::pollEvents() {
	_data.keyboardAction = false;
	_data.mouseAction = false;
	_data.keys.clear();

	glfwPollEvents();
	processKeyboard();

	if (!_data.keys.empty())
		_data.keyboardAction = true;

	auto currentTimeStamp = std::chrono::high_resolution_clock::now();
	_data.deltaTime = std::chrono::duration<float>(currentTimeStamp - _lastTimestamp).count();
	_lastTimestamp = currentTimeStamp;
	for (auto observer : _observers) {
		observer->updateInput(_data);
	}
}

void FPSCallbackManager::processKeyboard() {
	GLFWwindow* window = _window->getGlfwWindow();

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		_data.keys.push_back(Keyboard::Key::W);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		_data.keys.push_back(Keyboard::Key::S);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		_data.keys.push_back(Keyboard::Key::A);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		_data.keys.push_back(Keyboard::Key::D);
	}
}

void FPSCallbackManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// auto camera = reinterpret_cast<FPSCamera*>(glfwGetWindowUserPointer(window));

	switch (key) {
	case GLFW_KEY_ESCAPE:
		if(action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		break;
	case GLFW_KEY_P:
		if(action == GLFW_PRESS)
			_data.keys.push_back(Keyboard::Key::P);
		break;
	}

}

void FPSCallbackManager::mouseCallback(GLFWwindow* window, double xposIn, double yposIn) {
	_data.mouseAction = true;
	auto cbManager = reinterpret_cast<FPSCallbackManager*>(glfwGetWindowUserPointer(window));

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	static bool firstMouse = true;

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	_data.xoffset = xpos - lastX;
	_data.yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
}

void FPSCallbackManager::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	// auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}

float FPSCallbackManager::lastX = 0.0f;
float FPSCallbackManager::lastY = 0.0f;