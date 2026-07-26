#include "sampler_manager.h"

#include "vulkan/resource_manager/util.h"
#include "vulkan/wrapper/sampler/sampler.h"

std::unique_ptr<SamplerManager> SamplerManager::create() {
  return std::unique_ptr<SamplerManager>(new SamplerManager());
}

SamplerHandle SamplerManager::getOrCreateSampler(
    const LogicalDevice& logicalDevice, const SamplerMetadata& metadata) {
  auto [it, inserted] = _samplerCollisionMap.try_emplace(metadata);
  if (!inserted) {
    return it->second;
  }
  const SamplerHandle handle = getNextHandle(_samplerMap.size(), _freeSamplerHandles);
  _samplerMap.insertUnsafe(
      *handle,
      std::make_tuple(
          SamplerBuilder()
              .withFlags(metadata.flags)
              .withMinMagFilter(metadata.minFilter, metadata.magFilter)
              .withMipmapMode(metadata.mipmapMode)
              .withAddressMode(metadata.addressModeU, metadata.addressModeV, metadata.addressModeW)
              .withMipLodBias(metadata.mipLodBias)
              .withMaxAnisotropy(metadata.maxAnisotropy)
              .withCompareOp(metadata.compareOp)
              .withLodRange(metadata.minLod, metadata.maxLod)
              .withBorderColor(metadata.borderColor)
              .withUnnormalizedCoordinates(metadata.unnormalizedCoordinates)
              .buildSampler(logicalDevice),
          &it->first));
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

const SamplerMetadata& SamplerManager::getSamplerMetadata(SamplerHandle handle) const {
  return *std::get<const SamplerMetadata*>(_samplerMap.getValue(*handle));
}

std::tuple<const Sampler&, const SamplerMetadata&> SamplerManager::getSamplerWithMetadata(
    SamplerHandle handle) const {
  const std::tuple<Sampler, const SamplerMetadata*>& samplerWithMetadata =
      _samplerMap.getValue(*handle);
  return std::tie(std::get<Sampler>(samplerWithMetadata),
                  *std::get<const SamplerMetadata*>(samplerWithMetadata));
}
