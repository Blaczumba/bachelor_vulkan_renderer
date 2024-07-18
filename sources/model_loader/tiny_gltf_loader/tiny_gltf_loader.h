#pragma once

#include "model_loader/model_loader.h"
#include "primitives/primitives.h"

#include <tinygltf/tiny_gltf.h>

template<typename IndexType>
class TinyGLTFLoaderVertex : public ModelLoader<VertexPT> {
	tinygltf::TinyGLTF _loader;
public:
	VertexData<VertexPT, IndexType> extract(const std::string&) override;
};
