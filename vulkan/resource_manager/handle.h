#pragma once

#include "common/util/resource_handles.h"

#include "vulkan/wrapper/memory_objects/buffer.h"
#include "vulkan/wrapper/memory_objects/image.h"
#include "vulkan/wrapper/sampler/sampler.h"

namespace {

template <typename T> struct Handle;
template <> struct Handle<Sampler> { using type = SamplerHandle; };
template <> struct Handle<Buffer> { using type = GpuBufferHandle; };
template <> struct Handle<Image> { using type = GpuImageHandle; };

}  // namespace

template <typename T>
using HandleFor = typename Handle<T>::type;
