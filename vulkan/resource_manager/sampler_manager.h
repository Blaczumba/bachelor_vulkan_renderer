#pragma once

#include <tuple>
#include <unordered_map>

#include "common/util/resource_handles.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/hasher.h"
#include "vulkan/wrapper/sampler/sampler.h"

class SamplerManager {
  SamplerManager() noexcept = default;

public:
  static std::unique_ptr<SamplerManager> create();

  ~SamplerManager() = default;

  SamplerHandle getOrCreateSampler(
      const LogicalDevice& logicalDevice, const SamplerMetadata& metadata);

  void removeSampler(SamplerHandle handle);

  const Sampler& getSampler(SamplerHandle handle) const;

  const SamplerMetadata& getSamplerMetadata(SamplerHandle handle) const;

  std::tuple<const Sampler&, const SamplerMetadata&> getSamplerWithMetadata(
      SamplerHandle handle) const;

private:
  lib::SparseMap<std::tuple<Sampler, const SamplerMetadata*>, MAX_SAMPLERS> _samplerMap;
  std::vector<SamplerHandle> _freeSamplerHandles;

  std::unordered_map<SamplerMetadata, SamplerHandle, SamplerHasher> _samplerCollisionMap;
};
