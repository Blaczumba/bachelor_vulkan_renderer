#pragma once

#include <window/window.h>
#include <instance/instance.h>
#include <debug_messenger/debug_messenger.h>
#include <surface/surface.h>
#include <physical_device/device/physical_device.h>
#include <swapchain/swapchain.h>

class ApplicationBase {
protected:
    std::shared_ptr<Window> _window;
    std::shared_ptr<Instance> _instance;
    std::shared_ptr<Surface> _surface;
    std::shared_ptr<PhysicalDevice> _physicalDevice;
    std::shared_ptr<LogicalDevice> _logicalDevice;
    std::shared_ptr<Swapchain> _swapchain;

#ifdef VALIDATION_LAYERS_ENABLED
    std::shared_ptr<DebugMessenger> _debugMessenger;
#endif // VALIDATION_LAYERS_ENABLED

public:
    ApplicationBase();
    virtual ~ApplicationBase();
    virtual void run() = 0;
};
