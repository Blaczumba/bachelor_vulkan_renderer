#pragma once

#include <openxr/openxr.h>
#include <stdexcept>
#include <string_view>

class XrException : public std::runtime_error {
public:
  XrException(std::string_view msg, XrResult result)
    : std::runtime_error(msg.data()), _result(result) {}

  const char* what() const noexcept override {
    return std::runtime_error::what();
  }

private:
  XrResult _result;
};

#define CHECK_XRCMD(cmd, message) \
    if (XrResult result = cmd; result != XR_SUCCESS) throw XrException(message, result)
