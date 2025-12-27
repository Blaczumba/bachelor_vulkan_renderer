#pragma once

#include <array>
#include <span>

namespace lib {

template <typename Type, size_t N>
class InplaceVector {
public:
  InplaceVector() noexcept = default;

  ~InplaceVector() = default;

  size_t size() const;

  bool empty() const;

  Type& operator[](size_t index);

  const Type& operator[](size_t index) const;

  void push_back(const Type& value);

  void push_back(Type&& value);

  void pop_back();

  operator std::span<Type>() {
    return std::span<Type>(_data.get(), _size);
  }

  operator std::span<const Type>() const {
    return std::span<const Type>(_data.get(), _size);
  }

private:
  std::array<Type, N> _data;
  size_t _size = 0;
};

template <typename Type, size_t N>
size_t InplaceVector<Type, N>::size() const {
  return _size;
}

template <typename Type, size_t N>
bool InplaceVector<Type, N>::empty() const {
  return _size == 0;
}

template <typename Type, size_t N>
Type& InplaceVector<Type, N>::operator[](size_t index) {
  return _data[index];
}

template <typename Type, size_t N>
const Type& InplaceVector<Type, N>::operator[](size_t index) const {
  return _data[index];
}

template <typename Type, size_t N>
void InplaceVector<Type, N>::push_back(const Type& value) {
  _data[_size++] = value;
}

template <typename Type, size_t N>
void InplaceVector<Type, N>::push_back(Type&& value) {
  _data[_size++] = std::move(value);
}

template <typename Type, size_t N>
void InplaceVector<Type, N>::pop_back() {
  --_size;
}

}  // namespace lib
