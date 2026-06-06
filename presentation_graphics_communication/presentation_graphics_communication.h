#pragma once

#include "common/abstractions/contexts.h"
#include "lib/types/noncopyable.h"

#include <cstdint>
#include <initializer_list>
#include <vector>
#include <span>
#include <memory>

namespace engine {

// It is responsible for communication between main presentation and graphics context layers.
class PresentationGraphicsCommunication : private lib::Noncopyable {
  PresentationGraphicsCommunication() noexcept = default;

public:
  static std::unique_ptr<PresentationGraphicsCommunication> create();

  ~PresentationGraphicsCommunication() noexcept = default;

  void setCurrentSwapchainImageIndex(uint32_t swapchainImageIndex);

  uint32_t getCurrentSwapchainImageIndex() const;

  void setCameraContexts(std::span<const common::CameraContext> cameraContexts);

  void setCameraContexts(std::initializer_list<common::CameraContext> cameraContexts);

  std::span<const common::CameraContext> getCameraContexts() const;

private:
  uint32_t _swapchainImageIndex = 0;
  std::vector<common::CameraContext> _cameraContexts;
};

} // engine
