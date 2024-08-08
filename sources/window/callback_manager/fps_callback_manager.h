#pragma once

#include "callback_manager.h"

#include <iostream>
#include <functional>

class CallbackData;
class GLFWwindow;

class FPSCallbackManager : public CallbackManager {
public:
    FPSCallbackManager(std::shared_ptr<WindowGLFW> window);

    void pollEvents() override;

    static bool firstMouse;
    static float lastX;
    static float lastY;
private:
    static CallbackData _data;

    void processKeyboard();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

};
