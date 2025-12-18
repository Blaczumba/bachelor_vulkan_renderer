#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "file_loader.h"
#include "lib/buffer/buffer.h"

class StandardFileLoader : public FileLoader {
public:
  lib::Buffer<std::byte> loadFileToBuffer(std::string_view filePath) const override;

  std::string loadFileToString(std::string_view filePath) const override;

  ~StandardFileLoader() override = default;
};
