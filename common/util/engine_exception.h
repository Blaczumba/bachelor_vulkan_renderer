#pragma once

#include <stacktrace>
#include <stdexcept>
#include <string_view>

class EngineException : public std::runtime_error {
public:
  EngineException(std::string_view msg)
    : std::runtime_error(msg.data()), _stackTrace(std::stacktrace::current()) {}

  const char* what() const noexcept override {
    return std::runtime_error::what();
  }

  const std::stacktrace& stackTrace() const noexcept {
    return _stackTrace;
  }

private:
  std::stacktrace _stackTrace;
};
