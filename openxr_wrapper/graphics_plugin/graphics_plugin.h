#pragma once

#include <openxr/openxr.h>
#include <optional>
#include <span>

namespace xrw {

class GraphicsPlugin {
public:
  virtual ~GraphicsPlugin() = default;

  virtual std::span<const char* const> getOpenXrInstanceExtensions() const = 0;

  virtual const XrBaseInStructure* getGraphicsBinding() const = 0;

  virtual std::optional<int64_t> selectSwapchainFormat(
      std::span<const int64_t> runtimeFormats) const = 0;

  virtual void createSwapchainContext(
      XrSwapchain swapchain, int64_t format, uint32_t width, uint32_t height) = 0;

  virtual XrSwapchainImageBaseHeader* getSwapchainImages(XrSwapchain swapchain) = 0;

  virtual void initialize(XrInstance xrInstance, XrSystemId systemId) = 0;

  virtual void createResources() = 0;

  virtual void draw(const XrCompositionLayerProjectionView& projectionLayerView,
                    uint32_t swapchain_image_index) = 0;
};

}  // namespace xrw
