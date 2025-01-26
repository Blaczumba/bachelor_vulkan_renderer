#pragma once

#include <algorithm>
#include <memory>

template<typename T>
class Buffer {
	std::unique_ptr<T[]> _buffer;
	size_t _size;

public:
	Buffer() : _buffer(nullptr), _size(0) {}
	explicit Buffer(size_t size) : _buffer(std::make_unique_for_overwrite<T[]>(size)), _size(size) {}

	Buffer(const Buffer& other) : Buffer(other._size) {
		std::copy(other._buffer.get(), other._buffer.get() + _size, _buffer.get());
	}
	Buffer(Buffer&& other) : _buffer(std::move(other._buffer)), _size(other._size) {}

    Buffer& operator=(const Buffer& other) {
        if (this == &other) {
            return *this;
        }
        _size = other._size;
        _buffer = std::make_unique_for_overwrite<T[]>(_size);
        std::copy(other._buffer.get(), other._buffer.get() + _size, _buffer.get());
        return *this;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        _buffer = std::move(other._buffer);
        _size = other._size;
        other._size = 0;
        return *this;
    }

    T& operator[](size_t index) {
        return _buffer[index];
    }

    const T& operator[](size_t index) const {
        return _buffer[index];
    }

	T* get() { return _buffer.get(); }
	size_t size() const { return _size; }
};
