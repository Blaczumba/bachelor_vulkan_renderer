#pragma once

// #include <stacktrace>
#include <stdexcept>
#include <string_view>
#include <vulkan/vulkan.h>

class VkException : public std::runtime_error {
public:
  VkException(std::string_view msg, VkResult result)
    : std::runtime_error(msg.data()), _result(result)/*, _stackTrace(std::stacktrace::current())*/ {}

  const char* what() const noexcept override {
    return std::runtime_error::what();
  }

  VkResult getResult() const {
    return _result;
  }

//  const std::stacktrace& stackTrace() const noexcept {
//    return _stackTrace;
//  }

private:
  VkResult _result;
  // std::stacktrace _stackTrace;
};

#define CHECK_VKCMD(cmd, message) \
    if (VkResult result = cmd; result != VK_SUCCESS) [[unlikely]] throw VkException(message, result);
