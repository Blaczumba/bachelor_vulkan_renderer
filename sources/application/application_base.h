#pragma once

#include <window/window.h>
#include <instance/instance.h>
#include <debug_messenger/debug_messenger.h>
#include <surface/surface.h>
#include <physical_device/device/physical_device.h>
#include <render_pass/render_pass.h>
#include <swapchain/swapchain.h>
#include <command_buffer/command_buffer.h>
#include <render_pass/framebuffer/framebuffer.h>
#include <memory_objects/vertex_buffer.h>
#include <memory_objects/index_buffer.h>
#include <memory_objects/uniform_buffer.h>
#include <pipeline/graphics_pipeline.h>
#include <memory_objects/texture/texture_2D.h>
#include <model_loader/obj_loader/obj_loader.h>
#include <descriptor_set/descriptor_set.h>
#include <camera/fps_camera.h>
#include <window/callback_manager/fps_callback_manager.h>
#include <memory_objects/texture/texture_2D_depth.h>
#include <memory_objects/texture/texture_2D_color.h>
#include <memory_objects/texture/texture_2D_sampler.h>
#include <screenshot/screenshot.h>

class ApplicationBase {
protected:
    std::shared_ptr<Window> _window;
    std::shared_ptr<Instance> _instance;
    std::shared_ptr<DebugMessenger> _debugMessenger;
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
    virtual void draw() =0;
};
