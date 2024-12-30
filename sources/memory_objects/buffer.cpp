#include "buffer.h"

void BufferDeallocator::operator()(VmaWrapper& allocator, const VmaAllocation allocation) {
    allocator.destroyVkBuffer(buffer, allocation);
}
