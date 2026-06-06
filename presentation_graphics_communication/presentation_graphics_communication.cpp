#include "presentation_graphics_communication/presentation_graphics_communication.h"

#include <cstdint>
#include <vector>
#include <span>
#include <initializer_list>
#include <memory>

namespace engine {

std::unique_ptr<PresentationGraphicsCommunication>
PresentationGraphicsCommunication::create() {
  return std::unique_ptr<PresentationGraphicsCommunication>(
      new PresentationGraphicsCommunication());
}

void PresentationGraphicsCommunication::setCurrentSwapchainImageIndex(
    uint32_t swapchainImageIndex) {
  _swapchainImageIndex = swapchainImageIndex;
}

uint32_t PresentationGraphicsCommunication::getCurrentSwapchainImageIndex() const {
  return _swapchainImageIndex;
}

void PresentationGraphicsCommunication::setCameraContexts(
    std::span<const common::CameraContext> cameraContexts) {
  _cameraContexts.assign_range(cameraContexts);
}

void PresentationGraphicsCommunication::setCameraContexts(
    std::initializer_list<common::CameraContext> cameraContexts) {
  _cameraContexts.assign_range(cameraContexts);
}

std::span<const common::CameraContext> PresentationGraphicsCommunication::getCameraContexts() const {
  return _cameraContexts;
}

}  // namespace engine
