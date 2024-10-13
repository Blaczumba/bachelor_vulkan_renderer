#pragma once

#include "callback_manager.h"

#include <iostream>
#include <functional>

class CallbackData;
class GLFWwindow;
class WindowGLFW;

class FPSCallbackManager : public CallbackManager {
    static CallbackData _data;
    WindowGLFW* _window;

public:
    FPSCallbackManager(WindowGLFW* window);

    void pollEvents() override;

    static float lastX;
    static float lastY;

private:
    void processKeyboard();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};
