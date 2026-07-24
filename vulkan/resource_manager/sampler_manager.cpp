#include "sampler_manager.h"

#include "common/util/engine_exception.h"
#include "vulkan/resource_manager/util.h"

std::unique_ptr<SamplerManager> SamplerManager::create() {
  return std::unique_ptr<SamplerManager>(new SamplerManager());
}

SamplerHandle SamplerManager::transferSampler(Sampler&& sampler, const SamplerMetadata& metadata) {
  auto [it, inserted] = _samplerCollisionMap.try_emplace(metadata);
  if (!inserted) {
    return it->second;
  }
  SamplerHandle handle = getNextHandle(_samplerMap.size(), _freeSamplerHandles);
  _samplerMap.insertUnsafe(*handle, std::make_tuple(std::move(sampler), &it->first));
  return it->second = handle;
}

void SamplerManager::removeSampler(SamplerHandle handle) {
  _freeSamplerHandles.push_back(handle);
  _samplerCollisionMap.erase(*std::get<const SamplerMetadata*>(_samplerMap.getValue(*handle)));
  _samplerMap.eraseUnsafe(*handle);
}

const Sampler& SamplerManager::getSampler(SamplerHandle handle) const {
  return std::get<Sampler>(_samplerMap.getValue(*handle));
}
