#include "model_loader.h"

IndexTypeT getMatchingIndexType(size_t indicesCount) {
	if (indicesCount <= std::numeric_limits<uint8_t>::max()) {
		return IndexTypeT::UINT8;
	}
	else if (indicesCount <= std::numeric_limits<uint16_t>::max()) {
		return IndexTypeT::UINT16;
	}
	else if (indicesCount <= std::numeric_limits<uint32_t>::max()) {
		return IndexTypeT::UINT32;
	}
	return IndexTypeT::NONE;
}
