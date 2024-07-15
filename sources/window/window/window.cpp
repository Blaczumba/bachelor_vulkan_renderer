#include "window.h"

#include <iostream>

std::vector<const char*> Window::_windowExtensions = {};

const std::vector<const char*>& Window::getWindowExtensions() {
    return _windowExtensions;
}
