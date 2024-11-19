#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>

class Attachment {
public:
    enum class Type : uint8_t {
        COLOR_ATTACHMENT = 0,
        COLOR_ATTACHMENT_RESOLVE,
        DEPTH_ATTACHMENT
    };

    VkClearValue getClearValue() const { return _clearValue; }
    const VkAttachmentDescription& getDescription() const { return _description; }
    Type getAttachmentType() const { return _type; }
    VkImageLayout getSubpassImageLayout() const { return _subpassImageLayout; }

    constexpr Attachment(Type type, VkClearValue clearValue, VkAttachmentDescription description, VkImageLayout subpassImageLayout)
        : _type(type), _clearValue(clearValue), _description(description), _subpassImageLayout(subpassImageLayout) {}

private:
    VkClearValue _clearValue{};
    VkAttachmentDescription _description{};
    Type _type{};
    VkImageLayout _subpassImageLayout{};
};
