#include "window.h"

#include <iostream>

Window::Window(const Instance& instance) : _instance(instance) {}

Window::~Window() {
    vkDestroySurfaceKHR(_instance.getVkInstance(), _surface, nullptr);
}

const VkSurfaceKHR Window::getVkSurfaceKHR() const {
    return _surface;
}

const Instance& Window::getInstance() const {
    return _instance;
}
