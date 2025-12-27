#pragma once

#include <array>

#include "lib/types/util.h"

namespace lib {

template <typename Type, size_t N>
class SparseMap {
public:
  using IndexType = typename SmallestIndex<N>::type;

  SparseMap() noexcept = default;

  ~SparseMap() = default;

  IndexType size() const;

  bool exists(IndexType index) const;

  bool insert(IndexType index, Type value);

  void insertUnsafe(IndexType index, Type value);

  bool erase(IndexType index);

  void eraseUnsafe(IndexType index);

private:
  std::array<IndexType, N> _sparse;
  std::array<std::pair<Type, IndexType>, N> _dense;
  IndexType _size = 0;
};

template <typename Type, size_t N>
typename SparseMap<Type, N>::IndexType SparseMap<Type, N>::size() const {
  return _size;
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::exists(IndexType index) const {
  return index < N && _sparse[index] < _size && _dense[_sparse[index]].second == index;
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::insert(IndexType index, Type value) {
  if (_size == N || exists(index)) {
    return false;
  }

  _sparse[index] = _size;
  _dense[_size] = {value, _size};
  ++_size;
  return true;
}

template <typename Type, size_t N>
void SparseMap<Type, N>::insertUnsafe(IndexType index, Type value) {
  _sparse[index] = _size;
  _dense[_size] = {value, index};
  ++_size;
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::erase(IndexType index) {
  if (!exists(index)) {
    return false;
  }

  const IndexType denseIndex = _sparse[index];
  auto& [lastValue, lastIndex] = _dense[_size - 1];
  _dense[denseIndex] = {std::move(lastValue), lastIndex};
  _sparse[lastIndex] = denseIndex;
  --_size;
  return true;
}

template <typename Type, size_t N>
void SparseMap<Type, N>::eraseUnsafe(IndexType index) {
  const IndexType denseIndex = _sparse[index];
  auto& [lastValue, lastIndex] = _dense[_size - 1];
  _dense[denseIndex] = {std::move(lastValue), lastIndex};
  _sparse[lastIndex] = denseIndex;
  --_size;
}

}  // namespace lib
