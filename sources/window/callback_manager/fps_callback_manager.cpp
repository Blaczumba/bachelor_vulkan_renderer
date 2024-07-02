#include "fps_callback_manager.h"

FPSCallbackManager::FPSCallbackManager(std::shared_ptr<Window> window, FPSCamera& camera)
	: CallbackManager(window), camera(camera) {
	GLFWwindow* glfwWindow = _window->getGlfwWindow();
	glfwSetWindowUserPointer(glfwWindow, &camera);

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
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyboard(MovementDirections::FORWARD, 1.0f / 60.0f);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyboard(MovementDirections::BACKWARD, 1.0f / 60.0f);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(MovementDirections::LEFT, 1.0f / 60.0f);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(MovementDirections::RIGHT, 1.0f / 60.0f);
}

void FPSCallbackManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	auto camera = reinterpret_cast<FPSCamera*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void FPSCallbackManager::mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
	auto camera = reinterpret_cast<FPSCamera*>(glfwGetWindowUserPointer(window));

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

	camera->processMouseMovement(xoffset, yoffset);
}

void FPSCallbackManager::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	// auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}

bool FPSCallbackManager::firstMouse = true;
float FPSCallbackManager::lastX = 0.0f;
float FPSCallbackManager::lastY = 0.0f;