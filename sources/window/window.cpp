#include "window.h"

#include <iostream>

Window::Window(std::string_view name, uint32_t width, uint32_t height) {
    window = glfwCreateWindow(width, height, name.data(), nullptr, nullptr);

}

Window::~Window() {
    glfwDestroyWindow(window);
}

bool Window::closed() const {
    return glfwWindowShouldClose(window);
}

bool Window::open() const {
    return !glfwWindowShouldClose(window);
}

Extent Window::getFramebufferSize() const {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
}

GLFWwindow* Window::getGlfwWindow() {
    return window;
}
