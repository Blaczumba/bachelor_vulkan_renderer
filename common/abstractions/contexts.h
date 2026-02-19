#pragma once

#include <any>
#include <mutex>
#include <optional>
#include <span>
#include <variant>
#include <vector>

#include "common/camera/camera.h"
#include "common/util/resource_handles.h"

namespace common {

struct CameraContext {
  glm::vec3 position;
  glm::mat4 view;
  glm::mat4 proj;
};

struct DrawingContext {
  uint32_t imageIndex;
  std::vector<CameraContext> cameraContexts;

  std::mutex _mu;
};

enum class ObjectType : uint8_t {
  RENDERPASS = 0,
  FRAMEBUFFER,
  TEXTURE,
  BUFFER,
  PIPELINE,
};

struct AttachmentLayout {};

struct RenderpassCreateRequest {
  ObjectType type = ObjectType::RENDERPASS;
  std::optional<RenderpassHandle> returnHandle;

  void* nextChunk;
};

// We can create Renderpass on the fly when needed by
using RenderpassReference = std::variant<RenderpassCreateRequest*, RenderpassHandle>;

struct FramebufferCreateRequest {
  ObjectType type = ObjectType::FRAMEBUFFER;
  std::optional<FramebufferHandle> returnHandle;

  RenderpassReference renderpassRef;
  void* nextChunk;
};

struct UpdateContextResponse {};

struct UpdateContext {};

struct PresentResources {
  int64_t imageFormat;
  uint32_t width;
  uint32_t height;
  uint32_t numLayers;
  std::span<const std::byte> imageViews;  // Type erasure.
  bool multiview;
};

}  // namespace common
