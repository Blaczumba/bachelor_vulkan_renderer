#include "tiny_gltf_loader.h"

#include <stdexcept>

// TODO needs to be moved to .h

template<typename IndexType>
VertexData<Vertex, IndexType> TinyGLTFLoaderVertex<IndexType>::extract(const std::string& filePath) {
    tinygltf::Model model;

    std::string error;
    std::string warning;

    std::vector<Vertex> vertices;

    bool ret = _loader.LoadASCIIFromFile(&model, &error, &warning, filePath);
    // Use loader.LoadBinaryFromFile(&model, &err, &warn, filename); for .glb files

    if (!warning.empty()) {
        throw std::runtime_error(warning);
    }

    if (!error.empty()) {
        throw std::runtime_error(error);
    }

    if (!ret) {
        throw std::runtime_error("Failed to load gltf file.");
    }

    for (const auto& mesh : model.meshes) {
        for (const auto& primitive : mesh.primitives) {
            const auto& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
            const auto& posBufferView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posBufferView.buffer];

            const auto& colorAccessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
            const auto& colorBufferView = model.bufferViews[colorAccessor.bufferView];
            const auto& colorBuffer = model.buffers[colorBufferView.buffer];

            const auto& texCoordAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
            const auto& texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
            const auto& texCoordBuffer = model.buffers[texCoordBufferView.buffer];

            const float* posData = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);
            const float* colorData = reinterpret_cast<const float*>(&colorBuffer.data[colorBufferView.byteOffset + colorAccessor.byteOffset]);
            const float* texCoordData = reinterpret_cast<const float*>(&texCoordBuffer.data[texCoordBufferView.byteOffset + texCoordAccessor.byteOffset]);

            for (size_t i = 0; i < posAccessor.count; ++i) {
                vertices.emplace_back(
                    glm::vec3(posData[i * 3], posData[i * 3 + 1], posData[i * 3 + 2]),
                    glm::vec3(colorData[i * 3], colorData[i * 3 + 1], colorData[i * 3 + 2]),
                    glm::vec2(texCoordData[i * 2], texCoordData[i * 2 + 1])
                );
            }
        }
    }

    return { vertices };
}
