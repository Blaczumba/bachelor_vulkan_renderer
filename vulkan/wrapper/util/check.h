#pragma once

#include <stdexcept>
#include <vulkan/vulkan.h>

#include "common/status/status.h"

class VkException : public std::runtime_error {
public:
  VkException(std::string_view msg, VkResult result)
    : std::runtime_error(msg.data()), _result(result) {}

  const char* what() const noexcept override {
    return std::runtime_error::what();
  }

  VkResult getResult() const {
    return _result;
  }

private:
  VkResult _result;
};

#define CHECK_VKCMD(cmd, message) \
    if (VkResult result = cmd; result != VK_SUCCESS) [[unlikely]] throw VkException(message, VK_SUCCESS);
