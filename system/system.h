#pragma once

#include "common/abstractions/contexts.h"

#include <vector>
#include <span>

namespace system {

// Is responsible for communication between main layers of engine.
class System {
  System() = default;

public:
  static std::unique_ptr<System> create() {
    return std::unique_ptr<System>(new System());
  }

  ~System() = default;

  System(const System&) = delete;

  System(System&&) = delete;

  System& operator=(const System&) = delete;

  System& operator=(System&&) = delete;

  void setCurrentSwapchainImageIndex(uint32_t swapchainImageIndex) {
	_swapchainImageIndex = swapchainImageIndex;
  }

  uint32_t getCurrentSwapchainImageIndex() const {
    return _swapchainImageIndex;
  }

  void setCameraContexts(std::span<const common::CameraContext> cameraContexts) {
    _cameraContexts.assign_range(cameraContexts);
  }

  std::span<const common::CameraContext> getCameraContexts() const {
    return _cameraContexts;
  }

private:
  uint32_t _swapchainImageIndex;
  std::vector<common::CameraContext> _cameraContexts;
};

} // system
