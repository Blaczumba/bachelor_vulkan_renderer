#pragma once

#include <any>
#include <cstdint>

#include "common/abstractions/contexts.h"
#include "common/camera/camera.h"

namespace common {

enum class GraphicsApi : uint8_t {
  VULKAN = 0,
};

class GraphicsContext {
public:
  virtual ~GraphicsContext() = default;

  virtual UpdateContextResponse update(const UpdateContext& updateContext) = 0;

  virtual void draw(const DrawingContext& drawingContext) = 0;

  virtual void initializeResources() = 0;

  virtual void waitCompleteExecution() const = 0;

  virtual void createPresentingResources(const PresentResources& presentResources) = 0;

  virtual void waitDeviceIdle() const = 0;

  virtual std::any getSynchronizationContext() const = 0;

private:
};

}  // namespace common
