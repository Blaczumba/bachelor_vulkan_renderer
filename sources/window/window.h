#pragma once

#include "primitives/primitives.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>

class Window {
	GLFWwindow* window;
public:
	Window(std::string_view name, uint32_t width, uint32_t height);
	~Window();
	bool closed() const;
	bool open() const;

	void setWindowSize(int width, int height);
	Extent getFramebufferSize() const;
	GLFWwindow* getGlfwWindow();
};
