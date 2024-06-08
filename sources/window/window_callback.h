#pragma once

#include <GLFW/glfw3.h>

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    // auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
}