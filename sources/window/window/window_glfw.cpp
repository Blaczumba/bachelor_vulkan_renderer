#include "window_glfw.h"

#include <GLFW/glfw3.h>

#include <stdexcept>

WindowGLFW::WindowGLFW(std::string_view windowName, uint32_t width, uint32_t height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	_window = glfwCreateWindow(width, height, windowName.data(), nullptr, nullptr);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    _windowExtensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

WindowGLFW::~WindowGLFW() {
    glfwDestroyWindow(_window);

    glfwTerminate();
}

GLFWwindow* WindowGLFW::getGlfwWindow() {
    return _window;
}

VkSurfaceKHR WindowGLFW::createSurface(VkInstance instance) const {
    VkSurfaceKHR surface;

    if (glfwCreateWindowSurface(instance, _window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }

    return surface;
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