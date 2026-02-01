#pragma once

#include "vulkan/wrapper/sampler/sampler.h"
#include "lib/sparse/sparse_map.h"
#include "common/util/resource_handles.h"

class SamplerManager {
  SamplerManager() noexcept = default;

public:
  static std::unique_ptr<SamplerManager> create();

  ~SamplerManager() = default;

  SamplerHandle transferSampler(Sampler&& sampler);

  void removeSampler(SamplerHandle handle);

  const Sampler& getSampler(SamplerHandle handle) const;

private:
  using SamplerMap = lib::SparseMap<Sampler, MAX_SAMPLERS>;
  SamplerMap _samplerMap;
  std::vector<SamplerHandle> _freeSamplerHandles;
};
