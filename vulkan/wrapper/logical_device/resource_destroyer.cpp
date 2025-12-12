#include "resource_destroyer.h"

void ThreadedResourceDestroyer::destroyResource(std::function<void()>&& destroyResource) {
  _worker.addJob(std::move(destroyResource));
}

void ImmediateResourceDestroyer::destroyResource(std::function<void()>&& destroyResource) {
  destroyResource();
}
