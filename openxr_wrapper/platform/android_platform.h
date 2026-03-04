#pragma once

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <jni.h>
#ifdef XR_USE_GRAPHICS_API_VULKAN
  #include <vulkan/vulkan.h>
#endif
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <span>
#include <string_view>

#include "platform.h"

namespace xrw {

struct AndroidAppState {
  bool resumed = false;
};

class AndroidPlatform : public Platform {
public:
  explicit AndroidPlatform(struct android_app* app);

  std::span<const char* const> getInstanceExtensions() const override;

  const XrBaseInStructure* getInstanceCreateExtension() const override;

  bool shouldClose() const override;

  void pollPlatformEvents(bool applicationRunning) override;

private:
  struct android_app* _app;
  JNIEnv* _env;

  AndroidAppState _appState;

  XrInstanceCreateInfoAndroidKHR _instance_create_info_android;
};

}  // namespace xrw
