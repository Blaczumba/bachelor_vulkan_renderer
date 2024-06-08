#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>

class Window {
	GLFWwindow* window;
public:
	Window(std::string_view name, uint32_t width, uint32_t height);
	~Window();
	bool closed() const;
	bool running() const;
	VkExtent2D getFramebufferSize() const;
	GLFWwindow* getGlfwWindow();
};
