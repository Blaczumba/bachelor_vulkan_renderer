#include "openxr_wrapper/presentation/presentation.h"

#include "common/file/file_loader.h"
#include "openxr_wrapper/graphics_plugin/graphics_plugin_vulkan.h"  // Needs to be included before platform.
#include "openxr_wrapper/platform/android_platform.h"

#include <spdlog/spdlog.h>

namespace xrw {

namespace {

std::unique_ptr<GraphicsPlugin> createGraphicsPlugin(common::GraphicsApi graphicsApi) {
  switch (graphicsApi) {
    case common::GraphicsApi::VULKAN:
      return std::make_unique<GraphicsPluginVulkan>(nullptr);
  }
}

}  // namespace

Presentation::Presentation(std::unique_ptr<Platform>&& platform, std::unique_ptr<GraphicsPlugin>&& graphicsPlugin,
    std::unique_ptr<common::GraphicsContext>&& graphicsContext, std::unique_ptr<Instance>&& instance,
    std::unique_ptr<System>&& system, std::unique_ptr<Session>&& session, std::vector<Swapchain>&& swapchains) noexcept
    : _platform(std::move(platform)), _graphicsPlugin(std::move(graphicsPlugin)), _graphicsContext(std::move(graphicsContext)),
    _instance(std::move(instance)), _system(std::move(system)), _session(std::move(session)), _swapchains(std::move(swapchains)) {}

std::unique_ptr<common::Presentation> Presentation::create(
    std::unique_ptr<Platform>&& platform, common::GraphicsApi graphicsApi,
    const FileLoader& fileLoader) {
  std::unique_ptr<GraphicsPlugin> graphicsPlugin = createGraphicsPlugin(graphicsApi);
  std::unique_ptr<Instance> instance = Instance::create("BejzakEngine", *platform, *graphicsPlugin);
  std::unique_ptr<System> system = System::create(*instance);
  std::unique_ptr<common::GraphicsContext> graphicsContext = graphicsPlugin->createGraphicsContext(
      instance->getXrInstance(), system->getXrSystemId(), fileLoader);
  std::unique_ptr<Session> session = Session::create(*system, *graphicsPlugin);
  std::vector<Swapchain> swapchains =
      SwapchainBuilder()
          .withArraySize(2)
          .withViewConfigType(VIEW_CONFIG_TYPE)
          .build(*session, *graphicsPlugin);
  return std::unique_ptr<common::Presentation>(new Presentation(std::move(platform), std::move(graphicsPlugin), std::move(graphicsContext),
                                               std::move(instance), std::move(system), std::move(session), std::move(swapchains)));
}

void Presentation::run() {
  _graphicsContext->createPresentingResources(_graphicsPlugin->getSwapchainContext(_swapchains[0].getSwapchain()));
  _graphicsContext->initializeResources();
  _space = Space::create(_session->getXrSession(), XR_REFERENCE_SPACE_TYPE_LOCAL);
  while (!_platform->shouldClose()) {
    _platform->pollPlatformEvents(_sessionRunning);
    pollEvents();
    if (!_sessionRunning) {
      continue;
    }

    pollActions();
    // render
  }
}

common::GraphicsContext* Presentation::getGraphicsContext() {
  return _graphicsContext.get();
}

const XrEventDataBaseHeader* Presentation::tryReadNextEvent() {
  auto baseHeader =
      reinterpret_cast<XrEventDataBaseHeader *>(&_eventDataBuffer);
  baseHeader->type = XR_TYPE_EVENT_DATA_BUFFER;
  XrResult result =
      xrPollEvent(_instance->getXrInstance(), &_eventDataBuffer);
  if (result == XR_SUCCESS) {
    if (baseHeader->type == XR_TYPE_EVENT_DATA_EVENTS_LOST) {
      auto eventsLost = reinterpret_cast<XrEventDataEventsLost *>(baseHeader);
      spdlog::warn("{} events lost", eventsLost->lostEventCount);
    }
    return baseHeader;
  }
  if (result != XR_EVENT_UNAVAILABLE) {
    spdlog::error("xr pull event unknown result");
  }
  return nullptr;
}

void Presentation::handleSessionStateChangedEvent(const XrEventDataSessionStateChanged& stateChangedEvent) {
  if ((stateChangedEvent.session != XR_NULL_HANDLE) &&
      (stateChangedEvent.session != _session->getXrSession())) {
    spdlog::error("XrEventDataSessionStateChanged for unknown session");
    return;
  }
  _sessionState = stateChangedEvent.state;
  switch (_sessionState) {
    case XR_SESSION_STATE_READY: {
      const XrSessionBeginInfo sessionBeginInfo = {
          .type = XR_TYPE_SESSION_BEGIN_INFO,
          .primaryViewConfigurationType = VIEW_CONFIG_TYPE};

      xrBeginSession(_session->getXrSession(), &sessionBeginInfo);
      _sessionRunning = true;
      break;
    }
    case XR_SESSION_STATE_STOPPING: {
      _sessionRunning = false;
      xrEndSession(_session->getXrSession());
      break;
    }
    default:
      break;
  }
}

void Presentation::pollEvents() {
  while (const XrEventDataBaseHeader *event = tryReadNextEvent()) {
    switch (event->type) {
      case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING: {
        const auto& instanceLossPending =
            reinterpret_cast<const XrEventDataInstanceLossPending &>(*event);
        spdlog::warn("XrEventDataInstanceLossPending by {}",
                     instanceLossPending.lossTime);
        return;
      }
      case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED: {
        const auto &sessionStateChangedEvent =
            reinterpret_cast<const XrEventDataSessionStateChanged &>(*event);
        handleSessionStateChangedEvent(sessionStateChangedEvent);
        break;
      }
      case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED: {
      } break;
      case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
      default: {
        spdlog::debug("Ignoring event type");
        break;
      }
    }
  }
}

void Presentation::pollActions() {
  const XrActionStateGetInfo getInfo = {.type = XR_TYPE_ACTION_STATE_GET_INFO,
      .next = nullptr,
      .action = _input.quitAction,
      .subactionPath = XR_NULL_PATH};

  XrActionStateBoolean quitValue = {.type = XR_TYPE_ACTION_STATE_BOOLEAN};

  // TODO: CHECK_XRCMD
  xrGetActionStateBoolean(_session->getXrSession(), &getInfo, &quitValue);
  if ((quitValue.isActive == XR_TRUE) &&
      (quitValue.changedSinceLastSync == XR_TRUE) &&
      (quitValue.currentState == XR_TRUE)) {
    // TODO: CHECK_XRCMD
    xrRequestExitSession(_session->getXrSession());
  }
}

}  // namespace xrw
