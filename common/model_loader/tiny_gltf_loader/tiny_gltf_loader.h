#pragma once

#ifdef __ANDROID__
  #include <android/asset_manager.h>
#endif
#include <filesystem>
#include <format>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <tinygltf/tiny_gltf.h>
#include <vector>

#include "common/file/file.h"
#include "common/model_loader/model_loader.h"
#include "common/util/asset_manager.h"
#include "common/util/engine_exception.h"
#include "common/util/geometry.h"
#include "common/util/primitives.h"
#include "common/util/resource_handles.h"
#include "lib/buffer/shared_buffer.h"

#ifdef __ANDROID__
void setAssetmanager(AAssetManager* assetManager);
#endif

struct SharedData {
  tinygltf::Model model;
  std::vector<lib::Buffer<glm::vec3>> tangents;
};

namespace {

template <typename AssetManagerImpl>
void processNode(common::AssetManager<AssetManagerImpl>& assetManager,
                 std::shared_ptr<SharedData>& sharedData, const tinygltf::Node& node,
                 const glm::mat4& parentTransform, std::vector<VertexData>& vertexDataList,
                 std::unordered_map<std::string, StagingImageDataResourceHandle>& textureIndexMap,
                 const std::string& baseDir);

}  // namespace

template <typename AssetManagerImpl>
std::vector<VertexData> LoadGltfFromFile(
    common::AssetManager<AssetManagerImpl>& assetManager, const std::string& filePath) {
  auto sharedData = std::make_shared<SharedData>();
  tinygltf::TinyGLTF loader;
  if (!std::filesystem::exists(std::filesystem::path(filePath))) {
    throw EngineException(std::format("{} does not exists in the filesystem.", filePath));
  }

  if (filePath.ends_with(".glb")) {
    loader.LoadBinaryFromFile(&sharedData->model, nullptr, nullptr, filePath);
  } else if (filePath.ends_with(".gltf")) {
    loader.LoadASCIIFromFile(&sharedData->model, nullptr, nullptr, filePath);
  } else {
    throw EngineException(
        std::format("GLTF loader cannot load {} which is not .gltf or .glb format.", filePath));
  }

  const std::string baseDir = std::filesystem::path(filePath).parent_path().string();
  std::vector<VertexData> vertexDataList;
  std::unordered_map<std::string, StagingImageDataResourceHandle> textureIndexMap;
  for (const tinygltf::Scene& scene : sharedData->model.scenes) {
    for (int nodeIndex : scene.nodes) {
      const tinygltf::Node& node = sharedData->model.nodes[nodeIndex];

      processNode(assetManager, sharedData, node, glm::mat4(1.0f), vertexDataList, textureIndexMap,
                  baseDir);
    }
  }
  return vertexDataList;
}

template <typename AssetManagerImpl>
std::vector<VertexData> LoadGltfFromString(
    common::AssetManager<AssetManagerImpl>& assetManager, const std::string& dataString,
    const std::string& baseDir) {
  auto sharedData = std::make_shared<SharedData>();
  tinygltf::TinyGLTF loader;
  std::string error, warning;

  loader.LoadASCIIFromString(
      &sharedData->model, &error, &warning, dataString.data(), dataString.size(), baseDir);

  std::vector<VertexData> vertexDataList;
  std::unordered_map<std::string, StagingImageDataResourceHandle> textureIndexMap;
  for (const tinygltf::Scene& scene : sharedData->model.scenes) {
    for (int nodeIndex : scene.nodes) {
      const tinygltf::Node& node = sharedData->model.nodes[nodeIndex];
      processNode(assetManager, sharedData, node, glm::mat4(1.0f), vertexDataList, textureIndexMap,
                  baseDir);
    }
  }
  return vertexDataList;
}

namespace {

glm::mat4 GetNodeTransform(const tinygltf::Node& node) {
  glm::mat4 mat(1.0f);

  if (node.matrix.size() == 16) {
    mat = glm::make_mat4(node.matrix.data());
  } else {
    if (node.translation.size() == 3) {
      mat = glm::translate(
          mat, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
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

std::span<const unsigned char> processAttribute(
    const tinygltf::Model& model, std::map<std::string, int> attributes,
    const std::string& attribute) {
  auto it = attributes.find(attribute);
  if (it == attributes.cend()) {
    return {};
  }

  const tinygltf::Accessor& accessor = model.accessors[it->second];
  const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
  return std::span<const unsigned char>(
      &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count);
}

std::span<const std::byte> getIndices(
    const tinygltf::Model& model, const tinygltf::Primitive& primitive, uint8_t* indexSize) {
  const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
  const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

  size_t indicesCount = accessor.count;
  switch (accessor.componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      *indexSize = 1;
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      *indexSize = 2;
      break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      *indexSize = 4;
      break;
  }
  const size_t offset = accessor.byteOffset + bufferView.byteOffset;

  return std::span<const std::byte>(
      reinterpret_cast<const std::byte*>(&buffer.data[offset]), indicesCount * *indexSize);
}

std::string getTextureUri(const tinygltf::Model& model, const tinygltf::ParameterMap& values,
                          const std::string& textureType) {
  auto it = values.find(textureType);
  if (it == values.cend()) {
    return std::string{};
  }
  const tinygltf::Texture& texture = model.textures[it->second.TextureIndex()];
  const tinygltf::Image& image = model.images[texture.source];
  return image.uri;
}

template <typename AssetManagerImpl>
void processNode(common::AssetManager<AssetManagerImpl>& assetManager,
                 std::shared_ptr<SharedData>& sharedData, const tinygltf::Node& node,
                 const glm::mat4& parentTransform, std::vector<VertexData>& vertexDataList,
                 std::unordered_map<std::string, StagingImageDataResourceHandle>& textureIndexMap,
                 const std::string& baseDir) {
  const glm::mat4 currentTransform = parentTransform * GetNodeTransform(node);

  if (node.mesh < 0) {
    for (int childIndex : node.children) {
      processNode(assetManager, sharedData, sharedData->model.nodes[childIndex], currentTransform,
                  vertexDataList, textureIndexMap, baseDir);
    }
    return;
  }

  for (const tinygltf::Primitive& primitive : sharedData->model.meshes[node.mesh].primitives) {
    const std::map<std::string, int>& attributes = primitive.attributes;

    std::span<const unsigned char> positionsData =
        processAttribute(sharedData->model, attributes, "POSITION");
    lib::Buffer<glm::vec3> positions(
        reinterpret_cast<const glm::vec3*>(positionsData.data()), positionsData.size());
    std::span<const unsigned char> textureCoordsData =
        processAttribute(sharedData->model, attributes, "TEXCOORD_0");
    std::span<const unsigned char> normalsData =
        processAttribute(sharedData->model, attributes, "NORMAL");

    if (primitive.indices <= 0) {
      continue;
    }
    uint8_t indexSize;
    std::span<const std::byte> indicesBytes = getIndices(sharedData->model, primitive, &indexSize);

    std::string diffuseTexture;
    std::string metallicRoughnessTexture;
    std::string normalTexture;
    if (primitive.material >= 0) {
      const tinygltf::Material& material = sharedData->model.materials[primitive.material];
      diffuseTexture = getTextureUri(sharedData->model, material.values, "baseColorTexture");
      metallicRoughnessTexture =
          getTextureUri(sharedData->model, material.values, "metallicRoughnessTexture");
      normalTexture = getTextureUri(sharedData->model, material.additionalValues, "normalTexture");
    }

    if (diffuseTexture.empty() || metallicRoughnessTexture.empty() || normalTexture.empty()) {
      continue;
    }

    static std::pair<std::string, std::string> orders[] = {
      {"PTNT", "0123"},
      {"P",    "0"   }
    };

    sharedData->tangents.push_back(createTangents(
        indexSize, indicesBytes,
        std::span(reinterpret_cast<const glm::vec3*>(positionsData.data()), positionsData.size()),
        std::span(reinterpret_cast<const glm::vec2*>(textureCoordsData.data()),
                  textureCoordsData.size())));
    const StagingVertexDataResourceHandle vertexResourceID =
        assetManager.loadVertexDataInterleavingAsync(
            sharedData, indicesBytes, indexSize, orders,
            std::span(
                reinterpret_cast<const glm::vec3*>(positionsData.data()), positionsData.size()),
            std::span(reinterpret_cast<const glm::vec2*>(textureCoordsData.data()),
                      textureCoordsData.size()),
            std::span(reinterpret_cast<const glm::vec3*>(normalsData.data()), normalsData.size()),
            std::span(reinterpret_cast<const glm::vec3*>(sharedData->tangents.back().data()),
                      sharedData->tangents.back().size()));

    auto getOrLoadTexture = [&](const std::string& textureName) -> StagingImageDataResourceHandle {
      auto [it, inserted] = textureIndexMap.try_emplace(textureName);
      if (inserted) {
        it->second = assetManager.loadImageAsync(joinPaths(baseDir, textureName));
      }
      return it->second;
    };

    const StagingImageDataResourceHandle diffuseTextureID = getOrLoadTexture(diffuseTexture);
    const StagingImageDataResourceHandle normalTextureID = getOrLoadTexture(normalTexture);
    const StagingImageDataResourceHandle metallicRoughnessTextureID =
        getOrLoadTexture(metallicRoughnessTexture);

    vertexDataList.emplace_back(
        std::move(positions), indexSize, currentTransform,
        ImageID{diffuseTextureID, std::move(diffuseTexture)},
        ImageID{normalTextureID, std::move(normalTexture)},
        ImageID{metallicRoughnessTextureID, std::move(metallicRoughnessTexture)}, vertexResourceID);
  }

  for (int childIndex : node.children) {
    processNode(assetManager, sharedData, sharedData->model.nodes[childIndex], currentTransform,
                vertexDataList, textureIndexMap, baseDir);
  }
}

}  // namespace
