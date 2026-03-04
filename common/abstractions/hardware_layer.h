#pragma once

class HardwareAbstractionLayer {
public:
  virtual void initialize() = 0;

  virtual void run() = 0;

  virtual void shutdown() = 0;

  virtual ~HardwareAbstractionLayer() = default;
};
