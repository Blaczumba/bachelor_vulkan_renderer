#include "obj_loader.h"

template<>
VertexData<VertexPT, uint8_t> TinyOBJLoaderVertex::extract(const std::string& filePath) {
    return templatedExtractor<uint8_t>(filePath);
}

template<>
VertexData<VertexPT, uint16_t> TinyOBJLoaderVertex::extract(const std::string& filePath) {
    return templatedExtractor<uint16_t>(filePath);
}

template<>
VertexData<VertexPT, uint32_t> TinyOBJLoaderVertex::extract(const std::string& filePath) {
    return templatedExtractor<uint32_t>(filePath);
}