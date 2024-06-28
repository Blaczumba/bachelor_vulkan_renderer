#include "index_buffer.h"

//template<>
//VkIndexType IndexBuffer<uint8_t>::getIndexType() const {
//	return VK_INDEX_TYPE_UINT8_KHR;
//}

template<>
VkIndexType IndexBuffer<uint16_t>::getIndexType() const {
	return VK_INDEX_TYPE_UINT16;
}

template<>
VkIndexType IndexBuffer<uint32_t>::getIndexType() const {
	return VK_INDEX_TYPE_UINT32;
}