#pragma once

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "common/buffer/buffer.h"

size_t getShrunkIndexSize(std::span<const std::byte> indicesBuffer, size_t indexSize);
