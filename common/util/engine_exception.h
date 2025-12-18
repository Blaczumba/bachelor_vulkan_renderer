#pragma once

#include <stdexcept>
#include <string_view>

class EngineException : public std::runtime_error {
public:
  EngineException(std::string_view msg) : std::runtime_error(msg.data()) {}

  const char* what() const noexcept override {
    return std::runtime_error::what();
  }
};
