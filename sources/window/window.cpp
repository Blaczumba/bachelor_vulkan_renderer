#include "window.h"
#include "window_callback.h"

#include <iostream>

Window::Window(std::string_view name, uint32_t width, uint32_t height) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetKeyCallback(window, keyCallback);
}

Window::~Window() {
    glfwDestroyWindow(window);
}

bool Window::closed() const {
    return glfwWindowShouldClose(window);
}

bool Window::running() const {
    return !glfwWindowShouldClose(window);
}

VkOffset2D Window::getFramebufferSize() const {
    VkOffset2D extent;
    glfwGetFramebufferSize(window, &extent.x, &extent.y);
    return extent;
}

GLFWwindow* Window::getGlfwWindow() {
    return window;
}
