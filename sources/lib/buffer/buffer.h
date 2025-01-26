#pragma once

#include <algorithm>
#include <memory>

template<typename T>
class Buffer {
	std::unique_ptr<T> _buffer;
	size_t _size;

public:
	explicit Buffer(size_t size) : _buffer(std::make_unique_for_overwrite(size)), _size(size) {}

	Buffer(const Buffer& other) : Buffer(other._size) {
		std::copy(other._buffer.get(), other._buffer.get() + _size, _buffer.get());
	}
	Buffer(Buffer&& other) : _buffer(std::move(other._buffer)), _size(other._size) {}

	T* get() { return _buffer.get(); }
	size_t size() const { return size; }
};
