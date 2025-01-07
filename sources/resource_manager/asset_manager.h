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
#include <unordered_set>
#include <optional>

enum class CacheCode : uint8_t {
	NOT_CACHED,
	NOT_YET_PROCESSED,
	CACHED
};

namespace {

template<typename IndexType>
constexpr std::enable_if_t<std::is_unsigned<IndexType>::value, VkIndexType> getIndexType() {
	switch (sizeof(IndexType)) {
	case 1:
		return VK_INDEX_TYPE_UINT8_EXT;
	case 2:
		return VK_INDEX_TYPE_UINT16;
	case 4:
		return VK_INDEX_TYPE_UINT32;
	default:
		return VK_INDEX_TYPE_NONE_KHR;
	}
}

}

class AssetManager {
public:
	AssetManager(MemoryAllocator& memoryAllocator, ThreadPool* threadPool);
	CacheCode loadImage2D(std::string_view filePath);
	CacheCode loadImageCubemap(std::string_view filePath);
	template<typename VertexType, typename IndexType>
	CacheCode loadVertexData(std::string_view key, const std::vector<VertexType>& vertices, const std::vector<IndexType>& indices) {
		static_assert(VertexTraits<VertexType>::hasPosition, "Cannot load vertex data with no position defined");
		auto createStagingBuffer = [&](auto& data) {
			return StagingBuffer(_memoryAllocator, std::span(data.data(), data.size()));
		};
		StagingBuffer indexBuffer = createStagingBuffer(indices);
		auto handleVertexBuffer = [&]() -> std::tuple<std::optional<StagingBuffer>, StagingBuffer> {
			if (typeid(VertexType) == typeid(VertexP)) {
				return { std::nullopt, createStagingBuffer(vertices)};
			}
			else {
				std::vector<glm::vec3> primitives;
				primitives.reserve(vertices.size());
				std::transform(vertices.begin(), vertices.end(), std::back_inserter(primitives), [](const VertexType& vertex) { return vertex.pos; });
				return { createStagingBuffer(vertices), createStagingBuffer(primitives) };
			}
		};
		auto [vertexBuffer, primitivesVertexBuffer] = handleVertexBuffer();
		_vertexDataResources.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(key),
			std::forward_as_tuple(
				std::move(vertexBuffer),
				std::move(indexBuffer),
				getIndexType<IndexType>(),
				std::move(primitivesVertexBuffer),
				AABB{}
			)
		);

		return CacheCode::NOT_CACHED;
	}

	const std::pair<StagingBuffer, ImageDimensions>& getImageData(const std::string& filePath) const;
	CacheCode deleteImage(std::string_view filePath);

	struct VertexData {
		std::optional<StagingBuffer> vertexBuffer;	// std::nullopt when the type is VertexP.
		StagingBuffer indexBuffer;
		VkIndexType indexType;
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

	std::unordered_map<std::string, std::pair<StagingBuffer, ImageDimensions>> _imageResources; // TODO: change to image data probably.
	std::unordered_map<std::string, VertexData> _vertexDataResources;

	std::unordered_set<std::string> _awaitingImageResources;
	std::mutex _awaitingImageMutex;
};
