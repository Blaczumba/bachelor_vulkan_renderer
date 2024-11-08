#include "window_glfw.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

WindowGLFW::WindowGLFW(const Instance& instance, std::string_view windowName, uint32_t width, uint32_t height)
    : Window(instance) {
	_window = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);
    if (glfwCreateWindowSurface(_instance.getVkInstance(), _window, nullptr, &_surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

WindowGLFW::~WindowGLFW() {
    glfwDestroyWindow(_window);
}

GLFWwindow* WindowGLFW::getGlfwWindow() {
    return _window;
}

bool WindowGLFW::open() const {
    return !glfwWindowShouldClose(_window);
}

void WindowGLFW::setWindowSize(int width, int height) {
    glfwSetWindowSize(_window, width, height);
}

VkExtent2D WindowGLFW::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}