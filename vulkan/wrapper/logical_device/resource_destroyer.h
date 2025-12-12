#pragma once

#include <functional>

#include "lib/thread/worker.h"

class ResourceDestroyer {
public:
  virtual ~ResourceDestroyer() = default;

  virtual void destroyResource(std::function<void()>&& destroyResource) = 0;
};

class ThreadedResourceDestroyer : public ResourceDestroyer {
public:
  ThreadedResourceDestroyer() = default;

  ~ThreadedResourceDestroyer() override = default;

  void destroyResource(std::function<void()>&& destroyResource) override;

private:
  lib::thread::Worker _worker;
};

class ImmediateResourceDestroyer : public ResourceDestroyer {
public:
  ImmediateResourceDestroyer() = default;

  ~ImmediateResourceDestroyer() override = default;

  void destroyResource(std::function<void()>&& destroyResource) override;
};
