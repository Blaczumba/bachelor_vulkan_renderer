#pragma once

#include <algorithm>
#include <array>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace lib {

template <typename K, typename V, size_t N>
class AssociationList {
public:
  using value_type = std::pair<K, V>;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  constexpr iterator begin() noexcept {
    return _data.data();
  }
  constexpr iterator end() noexcept {
    return _data.data() + _size;
  }

  constexpr const_iterator begin() const noexcept {
    return _data.data();
  }
  constexpr const_iterator end() const noexcept {
    return _data.data() + _size;
  }

  constexpr const_iterator cbegin() const noexcept {
    return begin();
  }
  constexpr const_iterator cend() const noexcept {
    return end();
  }

  [[nodiscard]] constexpr bool empty() const noexcept {
    return _size == 0;
  }
  [[nodiscard]] constexpr size_t size() const noexcept {
    return _size;
  }
  [[nodiscard]] constexpr size_t max_size() const noexcept {
    return N;
  }

  [[nodiscard]] iterator find(const K& key) noexcept {
    return std::find_if(begin(), end(), [&](const value_type& p) {
      return p.first == key;
    });
  }

  [[nodiscard]] const_iterator find(const K& key) const noexcept {
    return std::find_if(cbegin(), cend(), [&](const value_type& p) {
      return p.first == key;
    });
  }

  [[nodiscard]] bool contains(const K& key) const noexcept {
    return find(key) != end();
  }

  V& at(const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("AssociationList::at");
  }

  const V& at(const K& key) const {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("AssociationList::at");
  }

  V& operator[](const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    return emplace_impl(key, V{}).first->second;
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    if (auto it = find(value.first); it != end()) {
      return {it, false};
    }
    return emplace_impl(value.first, value.second);
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    if (auto it = find(value.first); it != end()) {
      return {it, false};
    }
    return emplace_impl(std::move(value.first), std::move(value.second));
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) {
    if (auto it = find(key); it != end()) {
      return {it, false};
    }
    return emplace_impl(key, V(std::forward<Args>(args)...));
  }

  bool erase(const K& key) noexcept {
    if (auto it = find(key); it != end()) {
      erase(it);
      return true;
    }
    return false;
  }

  iterator erase(iterator it) noexcept {
    iterator last = end() - 1;
    if (it != last) {
      *it = std::move(*last);  // move last element
    }
    std::destroy_at(last);  // destroy last element (optional)
    --_size;
    return it;
  }

  void clear() noexcept {
    _size = 0;
  }

private:
  template <typename KK, typename VV>
  std::pair<iterator, bool> emplace_impl(KK&& key, VV&& value) {
    if (_size == N) [[unlikely]] {
      throw std::out_of_range("AssociationList capacity exceeded");
    }
    value_type* p = &_data[_size];
    std::construct_at(p, std::forward<KK>(key), std::forward<VV>(value));
    ++_size;
    return {p, true};
  }

  std::array<value_type, N> _data;  // TODO: change to std::inplace_vector when it's introduced.
  size_t _size{0};
};

template <typename K, typename V>
class DynamicAssociationList {
public:
  using value_type = std::pair<K, V>;
  using container_type = std::vector<value_type>;
  using iterator = typename container_type::iterator;
  using const_iterator = typename container_type::const_iterator;

  iterator begin() noexcept {
    return _data.begin();
  }
  iterator end() noexcept {
    return _data.end();
  }

  const_iterator begin() const noexcept {
    return _data.begin();
  }
  const_iterator end() const noexcept {
    return _data.end();
  }

  const_iterator cbegin() const noexcept {
    return _data.cbegin();
  }
  const_iterator cend() const noexcept {
    return _data.cend();
  }

  [[nodiscard]] bool empty() const noexcept {
    return _data.empty();
  }
  [[nodiscard]] size_t size() const noexcept {
    return _data.size();
  }

  void reserve(size_t n) {
    _data.reserve(n);
  }

  void shrink_to_fit() {
    _data.shrink_to_fit();
  }

  iterator find(const K& key) noexcept {
    return std::find_if(begin(), end(), [&](const value_type& p) {
      return p.first == key;
    });
  }

  const_iterator find(const K& key) const noexcept {
    return std::find_if(begin(), end(), [&](const value_type& p) {
      return p.first == key;
    });
  }

  [[nodiscard]] bool contains(const K& key) const noexcept {
    return find(key) != end();
  }

  V& at(const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("DynamicAssociationList::at");
  }

  const V& at(const K& key) const {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("DynamicAssociationList::at");
  }

  V& operator[](const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    return _data.emplace_back(key, V{}).second;
  }

  template <typename... Args>
  std::pair<iterator, bool> try_emplace(const K& key, Args&&... args) {
    if (auto it = find(key); it != end()) {
      return {it, false};
    }
    _data.emplace_back(key, V(std::forward<Args>(args)...));
    return {std::prev(_data.end()), true};
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    if (auto it = find(value.first); it != end()) {
      return {it, false};
    }
    _data.push_back(std::move(value));
    return {std::prev(_data.end()), true};
  }

  bool erase(const K& key) noexcept {
    if (auto it = find(key); it != end()) {
      erase(it);
      return true;
    }
    return false;
  }

  iterator erase(iterator it) noexcept {
    iterator last = std::prev(_data.end());
    if (it != last) {
      *it = std::move(*last);
    }
    _data.pop_back();
    return it;
  }

  void clear() noexcept {
    _data.clear();
  }

private:
  container_type _data;
};

}  // namespace lib
