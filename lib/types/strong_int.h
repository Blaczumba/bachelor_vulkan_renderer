#pragma once

#include <type_traits>

#define DEFINE_STRONG_INT(TYPE, INT_TYPE) using TYPE = lib::StrongInt<INT_TYPE, struct TYPE##Tag>

namespace lib {

template <typename T, typename Tag>
class StrongInt {
  static_assert(std::is_integral<T>::value, "StrongInt must be instantiated with an integer type");

  T _value;

public:
  StrongInt() noexcept = default;

  explicit StrongInt(T value) noexcept : _value(value) {}

  ~StrongInt() = default;

  T value() const noexcept {
    return _value;
  }

  T operator*() const noexcept {
    return _value;
  }

  bool operator==(const StrongInt& other) const noexcept {
    return _value == other._value;
  }

  bool operator!=(const StrongInt& other) const noexcept {
    return _value != other._value;
  }

  bool operator<(const StrongInt& other) const noexcept {
    return _value < other._value;
  }

  bool operator<=(const StrongInt& other) const noexcept {
    return _value <= other._value;
  }

  bool operator>(const StrongInt& other) const noexcept {
    return _value > other._value;
  }

  bool operator>=(const StrongInt& other) const noexcept {
    return _value >= other._value;
  }

  StrongInt& operator+=(const StrongInt& other) noexcept {
    _value += other._value;
    return *this;
  }

  StrongInt& operator-=(const StrongInt& other) noexcept {
    _value -= other._value;
    return *this;
  }

  StrongInt operator+(const StrongInt& other) const noexcept {
    return StrongInt(_value + other._value);
  }

  StrongInt operator-(const StrongInt& other) const noexcept {
    return StrongInt(_value - other._value);
  }

  StrongInt& operator++() noexcept {
    ++_value;
    return *this;
  }

  StrongInt operator++(int) noexcept {
    StrongInt temp = *this;
    ++_value;
    return temp;
  }
};

}  // namespace lib

namespace std {

template <typename T, typename Tag>
struct hash<lib::StrongInt<T, Tag>> {
  size_t operator()(const lib::StrongInt<T, Tag>& obj) const noexcept {
    return std::hash<T>{}(*obj);
  }
};

}  // namespace std
