#pragma once

#include "common/abstractions/presentation.h"
#include "common/abstractions/graphics_context.h"
#include "openxr_wrapper/graphics_plugin/graphics_plugin.h"

#include <memory>

namespace xrw {

class Presentation final : public common::Presentation {
  Presentation();

 public:
  static std::unique_ptr<common::Presentation> create(common::GraphicsApi graphicsApi, const FileLoader& fileLoader);

  ~Presentation() = default;

  common::GraphicsContext* getGraphicsContext() override;

  void run() override;

 private:
  std::unique_ptr<GraphicsPlugin> _graphicsPlugin;
  std::unique_ptr<common::GraphicsContext> _graphicsContext;
};

} // namespace xrw