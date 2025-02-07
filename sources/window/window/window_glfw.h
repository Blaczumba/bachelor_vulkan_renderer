#pragma once

#include "window.h"

#include <string_view>

class GLFWwindow;

class WindowGLFW : public Window {
	GLFWwindow* _window;

public:
	WindowGLFW(const Instance& instance, std::string_view windowName, uint32_t width, uint32_t height);
	~WindowGLFW();

	GLFWwindow* getGlfwWindow();

	virtual bool open() const override;
	virtual void setWindowSize(int width, int height) override;
	virtual VkExtent2D getFramebufferSize() const override;

};