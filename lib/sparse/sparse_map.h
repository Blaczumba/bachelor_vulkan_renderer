#pragma once

#include <array>
#include <span>

#include "lib/types/util.h"

namespace lib {

template <typename Type, size_t N>
class SparseMap {
public:
  using IndexType = typename SmallestIndex<N>::type;

  SparseMap() noexcept = default;

  ~SparseMap() = default;

  Type& getValue(IndexType index);

  const Type& getValue(IndexType index) const;

  std::span<Type> getValues();

  std::span<const Type> getValues() const;

  IndexType size() const;

  bool exists(IndexType index) const;

  bool insert(IndexType index, Type&& value);

  Type& insertUnsafe(IndexType index, Type&& value);

  bool erase(IndexType index);

  void eraseUnsafe(IndexType index);

private:
  IndexType _size = 0;
  std::array<IndexType, N> _sparse;
  std::array<IndexType, N> _dense;
  std::array<Type, N> _values;
};

template <typename Type, size_t N>
Type& SparseMap<Type, N>::getValue(IndexType index) {
  return _values[_sparse[index]];
}

template <typename Type, size_t N>
const Type& SparseMap<Type, N>::getValue(IndexType index) const {
  return _values[_sparse[index]];
}

template <typename Type, size_t N>
std::span<Type> SparseMap<Type, N>::getValues() {
  return std::span<Type>(_values.data(), _size);
}

template <typename Type, size_t N>
std::span<const Type> SparseMap<Type, N>::getValues() const {
  return std::span<const Type>(_values.data(), _size);
}

template <typename Type, size_t N>
typename SparseMap<Type, N>::IndexType SparseMap<Type, N>::size() const {
  return _size;
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::exists(IndexType index) const {
  return index < N && _sparse[index] < _size && _dense[_sparse[index]] == index;
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::insert(IndexType index, Type&& value) {
  if (_size == N || exists(index)) {
    return false;
  }

  _sparse[index] = _size;
  _dense[_size] = _size;
  _values[_size] = std::move(value);
  ++_size;
  return true;
}

template <typename Type, size_t N>
Type& SparseMap<Type, N>::insertUnsafe(IndexType index, Type&& value) {
  _sparse[index] = _size;
  _dense[_size] = index;
  return _values[_size++] = std::move(value);
}

template <typename Type, size_t N>
bool SparseMap<Type, N>::erase(IndexType index) {
  if (!exists(index)) {
    return false;
  }

  const IndexType denseIndex = _sparse[index];
  const IndexType lastIndex = _dense[--_size];
  _dense[denseIndex] = lastIndex;
  _values[denseIndex] = std::move(_values[lastIndex]);
  _sparse[lastIndex] = denseIndex;
  return true;
}

template <typename Type, size_t N>
void SparseMap<Type, N>::eraseUnsafe(IndexType index) {
  const IndexType denseIndex = _sparse[index];
  const IndexType lastIndex = _dense[--_size];
  _dense[denseIndex] = lastIndex;
  _values[denseIndex] = std::move(_values[lastIndex]);
  _sparse[lastIndex] = denseIndex;
}

}  // namespace lib
