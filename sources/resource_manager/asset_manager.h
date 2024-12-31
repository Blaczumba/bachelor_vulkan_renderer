#pragma once

#include "logical_device/logical_device.h"
#include "memory_objects/staging_buffer.h"
#include "model_loader/image_loader/image_loader.h"
#include "primitives/geometry.h"
#include "primitives/primitives.h"
#include "thread_pool/thread_pool.h"

#include <algorithm>
#include <atomic>
#include <iterator>
#include <mutex>
#include <string>
#include <string_view>
#include <span>
#include <unordered_map>
#include <optional>

enum class CacheCode : uint8_t {
	NOT_CACHED,
	CACHED
};

class AssetManager {
public:
	AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool);
	CacheCode loadImage2D(std::string_view filePath);
	CacheCode loadImageCubemap(std::string_view filePath);
	template<typename VertexType, typename IndexType>
	CacheCode loadVertexData(std::string_view key, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices) {
		static_assert(VertexTraits<VertexType>::hasPosition, "Cannot load vertex data with no position defined");
		StagingBuffer indexBuffer(_memoryAllocator, std::span(indices.data(), indices.size()));
		if (typeid(VertexType) == typeid(VertexP)) {
			StagingBuffer primitivesVertexBuffer(_memoryAllocator, std::span(vertices.data(), vertices.size()));
			_vertexDataResources.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::nullopt, std::move(indexBuffer), std::move(primitivesVertexBuffer), AABB{}));
			return CacheCode::NOT_CACHED;
		}
		StagingBuffer vertexBuffer(_memoryAllocator, std::span(vertices.data(), vertices.size()));
		std::vector<glm::vec3> primitives;
		primitives.reserve(vertices.size());
		std::transform(vertices.cbegin(), vertices.cend(), std::back_inserter(primitives), [](const VertexType& vertex) { return vertex.pos; });
		StagingBuffer primitivesVertexBuffer(_memoryAllocator, std::span(primitives.data(), primitives.size()));
		AABB aabb = createAABBfromVertices(primitives);
		_vertexDataResources.emplace(std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::move(vertexBuffer), std::move(indexBuffer), std::move(primitivesVertexBuffer), aabb));
		return CacheCode::NOT_CACHED;
	}
	const std::pair<StagingBuffer, ImageDimensions>& getImageData(const std::string& filePath) const;
	CacheCode deleteImage(std::string_view filePath);

	struct VertexData {
		std::optional<StagingBuffer> vertexBuffer;	// std::nullopt when the type is VertexP.
		StagingBuffer indexBuffer;
		StagingBuffer vertexBufferPrimitives;	// VertexP/glm::vec3 buffer.
		AABB aabb;
	};

	const VertexData& getVertexData(std::string_view key) const {
		auto ptr = _vertexDataResources.find(std::string{ key });
		return ptr->second; // TODO if not found branch
	}

private:
	MemoryAllocator& _memoryAllocator;
	ThreadPool* _threadPool;
	std::atomic<uint8_t> _index;
	std::mutex _mutex;

	std::unordered_map<std::string, std::pair<StagingBuffer, ImageDimensions>> _imageResources;
	std::unordered_map<std::string, VertexData> _vertexDataResources;
};
