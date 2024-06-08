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

VkExtent2D Window::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

GLFWwindow* Window::getGlfwWindow() {
    return window;
}
