#pragma once

#include "window.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string_view>

class WindowGLFW : public Window {
	GLFWwindow* _window;

	VkSurfaceKHR createSurface(VkInstance instance) const override;
public:
	WindowGLFW(std::string_view windowName, uint32_t width, uint32_t height);
	~WindowGLFW();

	GLFWwindow* getGlfwWindow();

	virtual bool open() const override;
	virtual void setWindowSize(int width, int height) override;
	virtual VkExtent2D getFramebufferSize() const override;

};