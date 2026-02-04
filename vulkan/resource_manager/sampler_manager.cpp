#include "sampler_manager.h"

#include "common/util/engine_exception.h"
#include "vulkan/resource_manager/util.h"

std::unique_ptr<SamplerManager> SamplerManager::create() {
  return std::unique_ptr<SamplerManager>(new SamplerManager());
}

SamplerHandle SamplerManager::transferSampler(Sampler&& sampler) {
  SamplerHandle handle = getNextHandle(_samplerMap.size(), _freeSamplerHandles);
  if (!_samplerMap.insert(*handle, std::move(sampler))) {
    throw EngineException("Failed to transfer the sampler into SamplerManager.");
  }

  return handle;
}

void SamplerManager::removeSampler(SamplerHandle handle) {
  _freeSamplerHandles.push_back(handle);
  _samplerMap.eraseUnsafe(*handle);
}

const Sampler& SamplerManager::getSampler(SamplerHandle handle) const {
  return _samplerMap.getValue(*handle);
}
