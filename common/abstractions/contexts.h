#pragma once

#include "common/camera/camera.h"
#include "common/util/resource_handles.h"

#include <any>
#include <mutex>
#include <span>
#include <variant>
#include <vector>
#include <optional>

namespace common {

struct DrawingContext {
  uint32_t imageIndex;
  Camera camera;

  std::mutex _mu;
};

enum class ObjectType : uint8_t {
  RENDERPASS = 0,
  FRAMEBUFFER,
  TEXTURE,
  BUFFER,
  PIPELINE,
};

struct AttachmentLayout {

};

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
  std::span<const std::byte> imageViews; // Type erasure.
  bool multiview;
};

}  // namespace common
