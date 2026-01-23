#include "file.h"

#include "common/util/engine_exception.h"

#include <format>

std::string_view getImageFileExtension(std::string_view filePath) {
  size_t lastDot = filePath.find_last_of(".");
  if (lastDot == std::string_view::npos) [[unlikely]] {
    throw EngineException(std::format("File {} does not contain an extension.", filePath));
  }

  return filePath.substr(lastDot);
}
