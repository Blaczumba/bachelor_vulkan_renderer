#pragma once

#include "common/file/file_loader.h"

#include <memory>

namespace common {

class Presentation {
public:
  virtual GraphicsContext* getGraphicsContext() = 0;

  virtual void run() = 0;

  virtual ~Presentation() = default;
};

}  // namespace common
