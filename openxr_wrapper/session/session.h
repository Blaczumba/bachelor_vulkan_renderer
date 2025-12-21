#pragma once

#include <memory>
#include <openxr/openxr.h>

#include "openxr_wrapper/platform/platform.h"
#include "openxr_wrapper/system/system.h"

namespace xrw {

class Session {
  XrSession _session;

  const System& _system;

  Session(XrSession session, const System& instance);

public:
  ~Session() = default;

  static std::unique_ptr<Session> create(const System& system, GraphicsPlugin& graphicsPlugin);

  XrSession getXrSession() const;

  const System& getSystem() const;
};

}  // namespace xrw
