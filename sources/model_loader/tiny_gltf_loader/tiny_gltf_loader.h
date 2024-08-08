#pragma once

#include "model_loader/model_loader.h"
#include "primitives/primitives.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

#include <vector>
#include <stdexcept>
#include <string>
#include <map>
#include <iostream>

glm::mat4 GetNodeTransform(const tinygltf::Node& node) {
    glm::mat4 mat(1.0f);

    if (node.matrix.size() == 16) {
        mat = glm::make_mat4(node.matrix.data());
    }
    else {
        if (node.translation.size() == 3) {
            mat = glm::translate(mat, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if (node.rotation.size() == 4) {
            glm::quat quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            mat *= glm::mat4_cast(quat);
        }
        if (node.scale.size() == 3) {
            mat = glm::scale(mat, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
    }

    return mat;
}

void ProcessNode(const tinygltf::Model& model, const tinygltf::Node& node, glm::mat4 parentTransform, std::vector<VertexData<VertexPTNTB, uint32_t>>& vertexDataList) {
    glm::mat4 currentTransform = parentTransform * GetNodeTransform(node);

    if (node.mesh >= 0) {
        const auto& mesh = model.meshes[node.mesh];

        VertexData<VertexPTNTB, uint32_t> vertexData;
        vertexData.model = currentTransform;

        for (const auto& primitive : mesh.primitives) {
            const auto& attributes = primitive.attributes;

            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> texCoords;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec3> tangents;
            std::vector<glm::vec3> bitangents;

            // Load positions
            if (attributes.find("POSITION") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("POSITION")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    positions.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }

            // Load texture coordinates
            if (attributes.find("TEXCOORD_0") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    texCoords.emplace_back(data[i * 2], data[i * 2 + 1]);
                }
            }

            // Load normals
            if (attributes.find("NORMAL") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    normals.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }

            // Load tangents and bitangents (optional)
            if (attributes.find("TANGENT") != attributes.end()) {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TANGENT")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* data = reinterpret_cast<const float*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                for (size_t i = 0; i < accessor.count; ++i) {
                    tangents.emplace_back(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
                }
            }

            // Compute bitangents (optional, if not provided)
            if (tangents.size() > 0 && normals.size() > 0 && texCoords.size() > 0) {
                for (size_t i = 0; i < positions.size(); ++i) {
                    glm::vec3 tangent = tangents[i];
                    glm::vec3 normal = normals[i];
                    glm::vec3 bitangent = glm::normalize(glm::cross(normal, tangent));
                    bitangents.push_back(bitangent);
                }
            }

            // Load indices
            if (primitive.indices >= 0) {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* data = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        vertexData.indices.push_back(data[i]);
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* data = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        vertexData.indices.push_back(data[i]);
                    }
                }
            }

            // Combine vertex attributes into VertexPTNTB
            for (size_t i = 0; i < positions.size(); ++i) {
                VertexPTNTB vertex;
                vertex.pos = positions[i];
                vertex.texCoord = (i < texCoords.size()) ? texCoords[i] : glm::vec2(0.0f, 0.0f);
                vertex.normal = (i < normals.size()) ? normals[i] : glm::vec3(0.0f, 0.0f, 0.0f);
                vertex.tangent = (i < tangents.size()) ? tangents[i] : glm::vec3(0.0f, 0.0f, 0.0f);
                vertex.bitangent = (i < bitangents.size()) ? bitangents[i] : glm::vec3(0.0f, 0.0f, 0.0f);
                vertexData.vertices.push_back(vertex);
            }

            // Load textures
            if (primitive.material >= 0) {
                const tinygltf::Material& material = model.materials[primitive.material];

                if (material.values.find("baseColorTexture") != material.values.end()) {
                    const tinygltf::Texture& texture = model.textures[material.values.at("baseColorTexture").TextureIndex()];
                    const tinygltf::Image& image = model.images[texture.source];
                    vertexData.diffuseTextures.push_back(image.uri);
                }
                if (material.values.find("metallicRoughnessTexture") != material.values.end()) {
                    const tinygltf::Texture& texture = model.textures[material.values.at("metallicRoughnessTexture").TextureIndex()];
                    const tinygltf::Image& image = model.images[texture.source];
                    vertexData.metallicRoughnessTextures.push_back(image.uri);
                }
                if (material.additionalValues.find("normalTexture") != material.additionalValues.end()) {
                    const tinygltf::Texture& texture = model.textures[material.additionalValues.at("normalTexture").TextureIndex()];
                    const tinygltf::Image& image = model.images[texture.source];
                    vertexData.normalTextures.push_back(image.uri);
                }
            }
        }

        vertexDataList.push_back(vertexData);
        // Calculate tangents and bitangents
        for (size_t i = 0; i < vertexDataList.back().indices.size(); i += 3) {
            VertexPTNTB& v0 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 0]];
            VertexPTNTB& v1 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 1]];
            VertexPTNTB& v2 = vertexDataList.back().vertices[vertexDataList.back().indices[i + 2]];

            glm::vec3 edge1 = v1.pos - v0.pos;
            glm::vec3 edge2 = v2.pos - v0.pos;
            glm::vec2 deltaUV1 = v1.texCoord - v0.texCoord;
            glm::vec2 deltaUV2 = v2.texCoord - v0.texCoord;

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

            glm::vec3 tangent = glm::normalize(f * (deltaUV2.y * edge1 - deltaUV1.y * edge2));
            glm::vec3 bitangent = glm::normalize(f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2));

            v0.tangent = tangent;
            v1.tangent = tangent;
            v2.tangent = tangent;

            v0.bitangent = bitangent;
            v1.bitangent = bitangent;
            v2.bitangent = bitangent;
        }
    }

    // Recursively process children nodes
    for (const auto& childIndex : node.children) {
        ProcessNode(model, model.nodes[childIndex], currentTransform, vertexDataList);
    }
}

std::vector<VertexData<VertexPTNTB, uint32_t>> LoadGLTF(const std::string& filePath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret;

    if (filePath.find(".glb") != std::string::npos || filePath.find(".bin") != std::string::npos)
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
    else if (filePath.find(".gltf") != std::string::npos)
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
    else
        ret = false;

    if (!ret) {
        throw std::runtime_error("Failed to load GLTF file: " + filePath + "\n" + err);
    }

    std::vector<VertexData<VertexPTNTB, uint32_t>> vertexDataList;

    // Process each scene in the model
    for (const auto& scene : model.scenes) {
        for (const auto& nodeIndex : scene.nodes) {
            const tinygltf::Node& node = model.nodes[nodeIndex];
            ProcessNode(model, node, glm::mat4(1.0f), vertexDataList);
        }
    }

    return vertexDataList;
}