#pragma once

#include <tuple>
#include <unordered_map>

#include "common/util/resource_handles.h"
#include "lib/sparse/sparse_map.h"
#include "vulkan/resource_manager/hasher.h"
#include "vulkan/resource_manager/ref.h"
#include "vulkan/resource_manager/reference_counter_with_hashing.h"
#include "vulkan/wrapper/sampler/sampler.h"

class SamplerManager final : public ReferenceCounterWithHashing<Sampler> {
  SamplerManager() = default;

public:
  static std::unique_ptr<SamplerManager> create();

  ~SamplerManager() = default;

  Ref<Sampler> getOrCreateSampler(
      const LogicalDevice& logicalDevice, const SamplerMetadata& metadata);
};
