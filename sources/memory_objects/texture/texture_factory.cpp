#include "texture_factory.h"

#include "logical_device/logical_device.h"
#include "texture_attachment.h"
#include "texture_2D_image.h"
#include "texture_2D_shadow.h"
#include "texture_cubemap.h"

#include <array>

std::unique_ptr<Texture> TextureFactory::createCubemap(const CommandPool& commandPool, std::string_view filePath, VkFormat format, float samplerAnisotropy) {
	return std::make_unique<TextureCubemap>(commandPool, filePath,
        Image{
		    .format = format,
		    .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
		    .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		    .layerCount = 6u
	    },
        Sampler{
		    .maxAnisotropy = samplerAnisotropy
	    }
    );
}

std::unique_ptr<Texture> TextureFactory::create2DShadowmap(const CommandPool& commandPool, uint32_t width, uint32_t height, VkFormat format) {
    return std::make_unique<Texture2DShadow>(commandPool,
        Image{
            .format = format,
            .width = width,
            .height = height,
            .aspect = VK_IMAGE_ASPECT_DEPTH_BIT,
            .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        },
        Sampler{
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
            .compareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
            .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
        }
    );
}

std::unique_ptr<Texture> TextureFactory::create2DTextureImage(const CommandPool& commandPool, std::string_view texturePath, VkFormat format, float samplerAnisotropy) {
    return std::make_unique<Texture2DImage>(commandPool, texturePath,
        Image{
            .format = format, .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
        },
        Sampler{
            .maxAnisotropy = samplerAnisotropy
        }
    );
}

std::unique_ptr<Texture> TextureFactory::createColorAttachment(const CommandPool& commandPool, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
    return std::make_unique<TextureAttachment>(commandPool, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        Image{
            .format = format,
            .width = extent.width,
            .height = extent.height,
            .aspect = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevels = 1,
            .numSamples = samples,
            .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        }
    );
}

bool hasStencil(VkFormat format) {
    const std::array<VkFormat, 4> formats = {
        VK_FORMAT_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D32_SFLOAT_S8_UINT
    };
    return std::find(formats.begin(), formats.end(), format) != std::end(formats);
}

std::unique_ptr<Texture> TextureFactory::createDepthAttachment(const CommandPool& commandPool, VkFormat format, VkSampleCountFlagBits samples, VkExtent2D extent) {
    const VkImageAspectFlags aspect = hasStencil(format) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    return std::make_unique<TextureAttachment>(commandPool, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        Image{
            .format = format,
            .width = extent.width,
            .height = extent.height,
            .aspect = aspect,
            .numSamples = samples,
            .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        }
    );
}
