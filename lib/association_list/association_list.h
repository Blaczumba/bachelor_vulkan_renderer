#include <algorithm>
#include <array>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace lib {

template <typename K, typename V, size_t N>
class AssociationList {
public:
  using value_type = std::pair<K, V>;
  using iterator = value_type*;
  using const_iterator = const value_type*;

  iterator begin() noexcept {
    return _data.data();
  }

  iterator end() noexcept {
    return _data.data() + _size;
  }

  const_iterator begin() const noexcept {
    return _data.data();
  }
  const_iterator end() const noexcept {
    return _data.data() + _size;
  }

  const_iterator cbegin() const noexcept {
    return _data.data();
  }
  const_iterator cend() const noexcept {
    return _data.data() + _size;
  }

  bool empty() const noexcept {
    return _size == 0;
  }

  size_t size() const noexcept {
    return _size;
  }

  size_t max_size() const noexcept {
    return N;
  }

  V& at(const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }

    throw std::out_of_range("AssociationList::at: key not found.");
  }

  const V& at(const K& key) const {
    if (auto it = find(key); it != end()) {
      return it->second;
    }

    throw std::out_of_range("AssociationList::at: key not found.");
  }

  V& operator[](const K& key) {
    auto it = find(key);
    if (it != end()) {
      return it->second;
    }

    if (_size >= N) {
      throw std::out_of_range("lib::AssociationList::operator[]: Capacity exceeded.");
    }

    _data[_size] = {key, V{}};
    return _data[_size++].second;
  }

  iterator find(const K& key) {
    return std::find_if(begin(), end(), [&](const auto& p) {
      return p.first == key;
    });
  }

  const_iterator find(const K& key) const {
    return std::find_if(begin(), end(), [&](const auto& p) {
      return p.first == key;
    });
  }

  bool contains(const K& key) const {
    return find(key) != end();
  }

  std::pair<iterator, bool> insert(const value_type& value) {
    auto it = find(value.first);
    if (it != end()) {
      return {it, false};
    }

    if (_size >= N) {
      throw std::out_of_range("lib::AssociationList::insert: Capacity exceeded");
    }

    _data[_size] = value;
    return {&_data[_size++], true};
  }

private:
  std::array<value_type, N> _data;
  size_t _size = 0;
};

template <typename K, typename V>
class DynamicAssociationList {
public:
  using value_type = std::pair<K, V>;
  using iterator = typename std::vector<value_type>::iterator;
  using const_iterator = typename std::vector<value_type>::const_iterator;

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

  bool empty() const noexcept {
    return _data.empty();
  }

  size_t size() const noexcept {
    return _data.size();
  }

  void reserve(size_t new_cap) {
    _data.reserve(new_cap);
  }

  V& at(const K& key) {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("AssociationList::at: key not found.");
  }

  const V& at(const K& key) const {
    if (auto it = find(key); it != end()) {
      return it->second;
    }
    throw std::out_of_range("AssociationList::at: key not found.");
  }

  V& operator[](const K& key) {
    auto it = find(key);
    if (it != end()) {
      return it->second;
    }

    _data.emplace_back(key, V{});
    return _data.back().second;
  }

  iterator find(const K& key) {
    return std::find_if(begin(), end(), [&](const auto& p) {
      return p.first == key;
    });
  }

  const_iterator find(const K& key) const {
    return std::find_if(begin(), end(), [&](const auto& p) {
      return p.first == key;
    });
  }

  bool contains(const K& key) const {
    return find(key) != end();
  }

  std::pair<iterator, bool> insert(value_type&& value) {
    auto it = find(value.first);
    if (it != end()) {
      return {it, false};
    }

    _data.push_back(std::move(value));
    return {std::prev(_data.end()), true};
  }

  bool erase(const K& key) {
    auto it = find(key);
    if (it == end()) {
      return false;
    }

    if (it != std::prev(end())) {
      *it = std::move(_data.back());
    }

    _data.pop_back();
    return true;
  }

  void clear() noexcept {
    _data.clear();
  }

private:
  std::vector<value_type> _data;
};

}  // namespace lib
