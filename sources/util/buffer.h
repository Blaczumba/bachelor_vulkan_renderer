#pragma once

#include <memory>

template<typename T>
class Buffer {
	std::unique_ptr<T> _buffer;
	size_t size;

public:
	Buffer(size_t size) : _buffer(std::make_unique_for_overwrite(size)), _size(size) {}
	T* get() { return _buffer.get(); }
};
