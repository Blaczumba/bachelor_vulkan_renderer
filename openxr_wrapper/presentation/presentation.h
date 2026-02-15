#pragma once

#include <memory>

#include "common/abstractions/graphics_context.h"
#include "common/abstractions/presentation.h"
#include "openxr_wrapper/graphics_plugin/graphics_plugin.h"
#include "openxr_wrapper/instance/instance.h"
#include "openxr_wrapper/session/session.h"
#include "openxr_wrapper/swapchain/swapchain.h"

namespace xrw {

struct AndroidAppState {
  void* applicationVm;
  void* applicationAcctivity;
  void* assetManager;
};

class Presentation final : public common::Presentation {
  Presentation();

public:
  static std::unique_ptr<common::Presentation> create(
      const AndroidAppState& androidAppState, common::GraphicsApi graphicsApi,
      const FileLoader& fileLoader);

  ~Presentation() = default;

  common::GraphicsContext* getGraphicsContext() override;

  void run() override;

private:
  static constexpr inline XrViewConfigurationType VIEW_CONFIG_TYPE =
      XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;

  std::unique_ptr<Platform> _platform;
  std::unique_ptr<GraphicsPlugin> _graphicsPlugin;
  std::unique_ptr<common::GraphicsContext> _graphicsContext;
  std::unique_ptr<Instance> _instance;
  std::unique_ptr<System> _system;
  std::unique_ptr<Session> _session;
  std::vector<Swapchain> _swapchains;
};

}  // namespace xrw
