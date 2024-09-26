#pragma once

#include "texture.h"
#include "texture_2D_color.h"
#include "texture_2D_depth.h"
#include "texture_2D_image.h"
#include "texture_2D_shadow.h"
#include "texture_cubemap.h"

#include <memory>

class TextureFactory {
public:
	template<typename T, typename... Args>
	std::shared_ptr<T> createTexture(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
};
