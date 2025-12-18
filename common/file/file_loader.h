#pragma once

#include <cstddef>
#include <string>
#include <string_view>

#include "lib/buffer/buffer.h"

class FileLoader {
public:
  virtual lib::Buffer<std::byte> loadFileToBuffer(std::string_view filePath) const = 0;

  virtual std::string loadFileToString(std::string_view filePath) const = 0;

  virtual ~FileLoader() = default;
};
