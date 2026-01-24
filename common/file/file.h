#pragma once

#include <string>
#include <string_view>

std::string_view getImageFileExtension(std::string_view filePath);

std::string joinPaths(std::string_view first, std::string_view second);

template <typename... T>
std::string joinPaths(T&&... paths) {
  size_t total_size = (paths.size() + ...) + (sizeof...(paths) - 1);
  std::string result;
  result.reserve(total_size);

  bool first = true;
  auto append_with_sep = [&](std::string_view v) {
    if (!first) {
      result.push_back('/');
    }

    result.append(v);
    first = false;
  };

  (append_with_sep(paths), ...);

  return result;
}
