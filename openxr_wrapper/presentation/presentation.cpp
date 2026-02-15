#include "openxr_wrapper/presentation/presentation.h"

#include "common/file/file_loader.h"
#include "openxr_wrapper/graphics_plugin/graphics_plugin_vulkan.h"  // Needs to be included before platform.
#include "openxr_wrapper/platform/android_platform.h"

namespace xrw {

namespace {

std::unique_ptr<GraphicsPlugin> createGraphicsPlugin(common::GraphicsApi graphicsApi) {
  switch (graphicsApi) {
    case common::GraphicsApi::VULKAN:
      return std::make_unique<GraphicsPluginVulkan>(nullptr);
  }
}

}  // namespace

std::unique_ptr<common::Presentation> Presentation::create(
    const AndroidAppState& androidAppState, common::GraphicsApi graphicsApi,
    const FileLoader& fileLoader) {
  AndroidData androidData = {androidAppState.applicationVm, androidAppState.applicationAcctivity};
  auto platform = std::make_unique<AndroidPlatform>(androidData);
  std::unique_ptr<GraphicsPlugin> graphicsPlugin = createGraphicsPlugin(graphicsApi);
  std::unique_ptr<Instance> instance = Instance::create("BejzakEngine", *platform, *graphicsPlugin);
  std::unique_ptr<System> system = System::create(*instance);
  std::unique_ptr<common::GraphicsContext> graphicsContext = graphicsPlugin->createGraphicsContext(
      instance->getXrInstance(), system->getXrSystemId(), fileLoader);
  std::unique_ptr<Session> session = Session::create(*system, *graphicsPlugin);
  std::vector<Swapchain> swapchains =
      SwapchainBuilder()
          .withArraySize(2)
          .withViewConfigType(VIEW_CONFIG_TYPE)
          .build(*session, *graphicsPlugin);

  // TODO: Right now we assume that there is only 1 swapchain.
  Swapchain& swapchain = swapchains[0];
  graphicsContext->createPresentingResources(
      graphicsPlugin->getSwapchainContext(swapchain.getSwapchain()));
  return nullptr;
}

void Presentation::run() {}

common::GraphicsContext* Presentation::getGraphicsContext() {
  return _graphicsContext.get();
}

}  // namespace xrw
