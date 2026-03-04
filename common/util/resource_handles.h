#pragma once

#include "lib/types/strong_int.h"
#include "lib/types/util.h"

// Bindless descriptor set indexing.
constexpr size_t MAX_UNIFORM_RESOURCES = 256;
DEFINE_STRONG_INT(UniformBufferHandle, lib::SmallestIndex<MAX_UNIFORM_RESOURCES>::type);
DEFINE_STRONG_INT(UniformTextureHandle, lib::SmallestIndex<MAX_UNIFORM_RESOURCES>::type);

constexpr size_t MAX_STAGING_IMAGE_DATA_RESOURCES = 256;
DEFINE_STRONG_INT(
    StagingImageDataResourceHandle, lib::SmallestIndex<MAX_STAGING_IMAGE_DATA_RESOURCES>::type);

constexpr size_t MAX_STAGING_VERTEX_DATA_RESOURCES = 256;
DEFINE_STRONG_INT(
    StagingVertexDataResourceHandle, lib::SmallestIndex<MAX_STAGING_VERTEX_DATA_RESOURCES>::type);

constexpr size_t MAX_GPU_BUFFERS = 1024;
DEFINE_STRONG_INT(GpuBufferHandle, lib::SmallestIndex<MAX_GPU_BUFFERS>::type);

constexpr size_t MAX_GPU_TEXTURES = 1024;
DEFINE_STRONG_INT(GpuTextureHandle, lib::SmallestIndex<MAX_GPU_TEXTURES>::type);

constexpr size_t MAX_SAMPLERS = 32;
DEFINE_STRONG_INT(SamplerHandle, lib::SmallestIndex<MAX_SAMPLERS>::type);

constexpr size_t MAX_FRAMEBUFFERS = 32;
DEFINE_STRONG_INT(FramebufferHandle, lib::SmallestIndex<MAX_FRAMEBUFFERS>::type);

constexpr size_t MAX_RENDERPASSES = 32;
DEFINE_STRONG_INT(RenderpassHandle, lib::SmallestIndex<MAX_RENDERPASSES>::type);
