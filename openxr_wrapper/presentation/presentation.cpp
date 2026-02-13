#include "openxr_wrapper/presentation/presentation.h"

//#include <openxr/openxr.h>
//#include <vulkan/vulkan.h>
//#include <openxr/openxr_platform.h>
#include "openxr_wrapper/graphics_plugin/graphics_plugin_vulkan.h"

namespace xrw {

namespace {

std::unique_ptr<GraphicsPlugin> createGraphicsPlugin(common::GraphicsApi graphicsApi) {
  switch(graphicsApi) {
    case common::GraphicsApi::VULKAN:
      return std::make_unique<GraphicsPluginVulkan>(nullptr);
  }
}

} // namespace

std::unique_ptr<common::Presentation> Presentation::create(common::GraphicsApi graphicsApi, const FileLoader& fileLoader) {

}

common::GraphicsContext* Presentation::getGraphicsContext() {
  return _graphicsContext.get();
}

} // namespace xrw