#pragma once

#include <memory>

#include "common/abstractions/graphics_context.h"
#include "common/file/file_loader.h"

namespace common {

class Presentation {
public:
  virtual GraphicsContext* getGraphicsContext() = 0;

  virtual void run() = 0;

  virtual ~Presentation() = default;
};

}  // namespace common
