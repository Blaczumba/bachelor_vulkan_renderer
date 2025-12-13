#pragma once

#include <memory>
#include <variant>
#include <vma/vk_mem_alloc.h>

#include "memory_allocator.h"

using MemoryAllocator = std::variant<VmaWrapper>;
using MemoryAllocatorPtr = std::unique_ptr<MemoryAllocator>;
using Allocation = std::variant<std::monostate, VmaAllocation>;
