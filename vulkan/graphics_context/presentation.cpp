#include "vulkan/graphics_context/presentation.h"

#include "common/util/engine_exception.h"
#include "lib/types/util.h"
#include "vulkan/graphics_context/graphics_context.h"
#include "vulkan/wrapper/instance/instance.h"
#include "vulkan/wrapper/logical_device/logical_device.h"
#include "vulkan/wrapper/physical_device/physical_device.h"
#include "vulkan/wrapper/surface/surface.h"
#include "vulkan/wrapper/swapchain/swapchain.h"

namespace vlkn {

Presentation::Presentation(
    std::shared_ptr<Window>&& window, Surface&& surface, Swapchain&& swapchain,
    std::unique_ptr<GraphicsContext<false, false>>&& graphicsContext, const FileLoader& fileLoader)
  : _window(std::move(window)), _surface(std::move(surface)), _swapchain(std::move(swapchain)),
    _graphicsContext(std::move(graphicsContext)),
    _mouseKeyboardManager(_window->createMouseKeyboardManager()) {}

std::unique_ptr<common::Presentation> Presentation::create(
    std::shared_ptr<Window>&& window, const FileLoader& fileLoader) {
  std::vector<const char*> requiredExtensions = window->getVulkanExtensions();
#ifdef VALIDATION_LAYERS_ENABLED
  requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif  // VALIDATION_LAYERS_ENABLED
  requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
  std::unique_ptr<Instance> instance =
      Instance::createPtr("Bejzak Engine", requiredExtensions, debugCallback1);
#ifdef VALIDATION_LAYERS_ENABLED
  DebugMessenger debugMessenger = DebugMessenger::create(*instance, debugCallback1);
#else
  DebugMessenger debugMessenger;
#endif  // VALIDATION_LAYERS_ENABLED
  Surface surface = Surface::create(*instance, *window);
  std::unique_ptr<PhysicalDevice> physicalDevice =
      PhysicalDevice::create(*instance, surface.getVkSurface());
  std::unique_ptr<LogicalDevice> logicalDevice = LogicalDevice::createPtr(*physicalDevice);

  const auto [width, height] = window->getFramebufferSize();
  Swapchain swapchain =
      SwapchainBuilder()
          .withPreferredPresentMode(VK_PRESENT_MODE_MAILBOX_KHR)
          .build(*logicalDevice, surface.getVkSurface(), VkExtent2D{width, height});

  auto graphicsContext =
      lib::dynamicUniqueCast<GraphicsContext<false, false>>(GraphicsContext<false, false>::create(
          std::move(instance), std::move(debugMessenger), std::move(physicalDevice),
          std::move(logicalDevice), fileLoader));
  return std::unique_ptr<Presentation>(
      new Presentation(std::move(window), std::move(surface), std::move(swapchain),
                       std::move(graphicsContext), fileLoader));
}

common::GraphicsContext* Presentation::getGraphicsContext() {
  return _graphicsContext.get();
}

void Presentation::run() {
  if (_graphicsContext == nullptr) {
    throw EngineException("Graphics context must be created before running presentation.");
  }

  std::chrono::steady_clock::time_point previous, current;
  float deltaTime;
  VkResult result;
  // This should be handled outside by engine but for now it is ok here:
  // _mouseKeyboardManager->absorbCursor();
  _mouseKeyboardManager->setKeyboardCallback([&](Keyboard::Key key, int action) {
    switch (key) {
      case Keyboard::Key::Escape:
        _window->close();
        break;
    }
  });
  /////////////////////////
  const SynchronizationContext* synchContext =
      std::any_cast<const SynchronizationContext*>(_graphicsContext->getSynchronizationContext());

  const auto [width, height] = _swapchain.getExtent();
  std::span<const VkImageView> imageViews = _swapchain.getImageViews();
  _graphicsContext->createPresentingResources(common::PresentResources{
    .imageFormat = static_cast<int64_t>(_swapchain.getVkFormat()),
    .width = width,
    .height = height,
    .numLayers = 1,
    .imageViews =
        std::span(reinterpret_cast<const std::byte*>(imageViews.data()), imageViews.size()),
    .multiview = false,
  });

  _graphicsContext->initializeResources();
  Camera camera(PerspectiveProjection{glm::radians(45.0f), 1920.0f / 1080.f, 0.01f, 500.0f},
                glm::vec3(0.0f), 5.5f, 0.01f);
  glm::mat4 tempViewPos(1.0f);
  while (_window->open()) {
    current = std::chrono::steady_clock::now();
    deltaTime = std::chrono::duration<float>(current - previous).count();
    previous = current;
    _window->pollEvents();
    // This should be handled outside by engine but for now it is ok here:
    camera.updateFromKeyboard(*_mouseKeyboardManager, deltaTime);
    /////////////////////////
    _graphicsContext->waitCompleteExecution();
    _swapchain.acquireNextImage(synchContext->imageAvailableSemaphores[synchContext->currentFrame],
                                &_drawingContext.imageIndex);
    _drawingContext.cameraContexts = {
      {
       camera.getPosition(),
       _mouseKeyboardManager->isPressed(Keyboard::Key::R) ? tempViewPos = camera.getViewMatrix() :
                                                             tempViewPos,
       camera.getProjectionMatrix(),
       }
    };
    _drawingContext.screenSpaceViewPos = _mouseKeyboardManager->getMousePosition();
    _graphicsContext->draw(_drawingContext);
    _swapchain.present(_drawingContext.imageIndex,
                       synchContext->renderFinishedSemaphores[_drawingContext.imageIndex]);
  }
  _graphicsContext->waitDeviceIdle();
}

}  // namespace vlkn
