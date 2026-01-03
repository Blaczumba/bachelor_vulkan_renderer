#pragma once

#include <array>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace lib {

template <typename T, size_t N>
class InplaceVector {
public:
  using value_type = T;
  using size_type = size_t;
  using iterator = T*;
  using const_iterator = const T*;

  constexpr InplaceVector() noexcept = default;

  ~InplaceVector() = default;

  [[nodiscard]] constexpr size_type size() const noexcept {
    return _size;
  }

  [[nodiscard]] constexpr size_type capacity() const noexcept {
    return N;
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return _size == 0;
  }

  constexpr iterator begin() noexcept {
    return data();
  }

  constexpr iterator end() noexcept {
    return data() + _size;
  }

  constexpr const_iterator begin() const noexcept {
    return data();
  }

  constexpr const_iterator end() const noexcept {
    return data() + _size;
  }

  T& operator[](size_type i) {
    if (i >= _size) {
      throw std::out_of_range("InplaceVector index out of range");
    }

    return _data[i];
  }

  const T& operator[](size_type i) const {
    if (i >= _size) {
      throw std::out_of_range("InplaceVector index out of range");
    }

    return _data[i];
  }

  T& front() {
    if (empty()) {
      throw std::logic_error("InplaceVector is empty");
    }

    return _data[0];
  }

  const T& front() const {
    if (empty()) {
      throw std::logic_error("InplaceVector is empty");
    }

    return _data[0];
  }

  T& back() {
    if (empty()) {
      throw std::logic_error("InplaceVector is empty");
    }

    return _data[_size - 1];
  }

  const T& back() const {
    if (empty()) {
      throw std::logic_error("InplaceVector is empty");
    }

    return _data[_size - 1];
  }

  void push_back(const T& value) {
    if (_size >= N) {
      throw std::out_of_range("InplaceVector capacity exceeded");
    }

    _data[_size++] = value;
  }

  void push_back(T&& value) {
    if (_size >= N) {
      throw std::out_of_range("InplaceVector capacity exceeded");
    }

    _data[_size++] = std::move(value);
  }

  void pop_back() {
    if (empty()) {
      throw std::logic_error("InplaceVector underflow");
    }

    destroy_at(--_size);
  }

  iterator erase_swap(iterator it) {
    if (it < begin() || it >= end()) {
      throw std::out_of_range("Invalid iterator in erase_swap");
    }

    if (it != end() - 1) {
      *it = std::move(back());
    }

    pop_back();
    return it;
  }

  T* data() noexcept {
    return _data.data();
  }

  const T* data() const noexcept {
    return _data.data();
  }

private:
  void destroy_at(size_type index) noexcept {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      _data[index] = T{};
    }
  }

  std::array<T, N> _data{};
  size_type _size{0};
};

}  // namespace lib
