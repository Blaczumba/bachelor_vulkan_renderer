#pragma once

#include <debug_messenger/debug_messenger.h>
#include <instance/instance.h>
#include <logical_device/logical_device.h>
#include <physical_device/physical_device.h>
#include <surface/surface.h>
#include <swapchain/swapchain.h>
#include <window/window/window_glfw.h>

class ApplicationBase {
protected:
    std::shared_ptr<WindowGLFW> _window;
    std::shared_ptr<Instance> _instance;

#ifdef VALIDATION_LAYERS_ENABLED
    std::shared_ptr<DebugMessenger> _debugMessenger;
#endif // VALIDATION_LAYERS_ENABLED

    std::shared_ptr<Surface> _surface;
    std::unique_ptr<PhysicalDevice> _physicalDevice;
    std::unique_ptr<LogicalDevice> _logicalDevice;
    std::unique_ptr<Swapchain> _swapchain;

public:
    ApplicationBase();
    virtual ~ApplicationBase();
    virtual void run() = 0;
};
