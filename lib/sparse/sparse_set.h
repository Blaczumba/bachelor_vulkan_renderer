#pragma once

#include <array>

#include "lib/types/util.h"

namespace lib {

template <size_t N>
class SparseSet {
public:
  using IndexType = typename SmallestIndex<N>::type;

  SparseSet() noexcept = default;

  ~SparseSet() = default;

  IndexType size() const;

  bool exists(IndexType index) const;

  bool insert(IndexType index);

  void insertUnsafe(IndexType index);

  bool erase(IndexType index);

  void eraseUnsafe(IndexType index);

private:
  std::array<IndexType, N> _sparse;
  std::array<IndexType, N> _dense;
  IndexType _size = 0;
};

template <size_t N>
typename SparseSet<N>::IndexType SparseSet<N>::size() const {
  return _size;
}

template <size_t N>
bool SparseSet<N>::exists(IndexType index) const {
  return index < N && _sparse[index] < _size && _dense[_sparse[index]] == index;
}

template <size_t N>
bool SparseSet<N>::insert(IndexType index) {
  if (_size == N || exists(index)) {
    return false;
  }

  _sparse[index] = _size;
  _dense[_size] = index;
  ++_size;
  return true;
}

template <size_t N>
void SparseSet<N>::insertUnsafe(IndexType index) {
  _sparse[index] = _size;
  _dense[_size] = index;
  ++_size;
}

template <size_t N>
bool SparseSet<N>::erase(IndexType index) {
  if (!exists(index)) {
    return false;
  }

  const IndexType denseIndex = _sparse[index];
  const IndexType lastIndex = _dense[_size - 1];
  _dense[denseIndex] = lastIndex;
  _sparse[lastIndex] = denseIndex;
  --_size;
  return true;
}

template <size_t N>
void SparseSet<N>::eraseUnsafe(IndexType index) {
  const IndexType denseIndex = _sparse[index];
  const IndexType lastIndex = _dense[_size - 1];
  _dense[denseIndex] = lastIndex;
  _sparse[lastIndex] = denseIndex;
  --_size;
}

}  // namespace lib
