#pragma once

#include <cstdint>
#include <limits>
#include <type_traits>

namespace lib {

template <size_t N>
struct SmallestIndex {
  using type = std::conditional_t<
      (N <= std::numeric_limits<uint8_t>::max()), uint8_t,
      std::conditional_t<(N <= std::numeric_limits<uint16_t>::max()), uint16_t, uint32_t>>;
};

}  // namespace lib
