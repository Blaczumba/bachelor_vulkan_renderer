#pragma once

#include "common/util/resource_handles.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/wrapper/sampler/sampler.h"

class SamplerManager {
  SamplerManager() noexcept = default;

public:
  static std::unique_ptr<SamplerManager> create();

  ~SamplerManager() = default;

  SamplerHandle transferSampler(Sampler&& sampler);

  void removeSampler(SamplerHandle handle);

  const Sampler& getSampler(SamplerHandle handle) const;

private:
  lib::SparseMap<Sampler, MAX_SAMPLERS> _samplerMap;
  std::vector<SamplerHandle> _freeSamplerHandles;
};
