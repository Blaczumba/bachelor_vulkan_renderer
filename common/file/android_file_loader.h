#pragma once

#include <android/asset_manager.h>
#include <string>

#include "file_loader.h"
#include "lib/buffer/buffer.h"

class AndroidFileLoader : public FileLoader {
public:
  AndroidFileLoader(AAssetManager* assetManager);

  ~AndroidFileLoader() override = default;

  lib::Buffer<std::byte> loadFileToBuffer(std::string_view filePath) const override;

  std::string loadFileToString(std::string_view filePath) const override;

private:
  AAssetManager* _assetManager;
};
