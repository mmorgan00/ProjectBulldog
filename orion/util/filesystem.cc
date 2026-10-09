#include "orion/util/filesystem.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

#include <fmt/format.h>

#include <filesystem>
#include <ios>
#include <iostream>
#include <string>

std::string Filesystem::get_exec_path() {
  std::filesystem::path exe_path;
#ifdef PLATFORM_LINUX
  // Linux: Read /proc/self/exe symlink
  char buffer[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
  if (count == -1) {
    throw std::runtime_error(
        fmt::format("Failed to resolve /proc/self/exe: {}", strerror(errno)));
  }
  buffer[count] = '\0';
  exe_path = std::filesystem::path(buffer);
// Windows: GetModuleFileNameW
#elif defined(PLATFORM_WINDOWS)
  wchar_t buffer[MAX_PATH];
  DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);

  if (length == 0 || length == MAX_PATH) {
    throw std::runtime_error("Failed to resolve executable path on Windows.");
  }

  exe_path = std::filesystem::path(buffer);

#else
  throw std::runtime_error("Unsupported Platform");
#endif
  return exe_path;
};

bool File::resource(const std::string& filename, File* out_file) {
  // Naive assignment. Real path is derived from `get_path()``
  out_file->base_path = filename;
  OE_LOG(FILESYSTEM, INFO, "Creating resource file handle res://{}", filename);
  out_file->handle.open(filename, std::ios::in | std::ios::out);
  if (out_file->handle.is_open()) {
    OE_LOG(FILESYSTEM, INFO, "Opened file res://{} ", filename);
    return true;
  }
  OE_LOG(FILESYSTEM, INFO, "Failed to open file res://{} ", filename);
  return false;
};

// TODO: Text vs binary. Now you've done it. Guess we're just supporting text
// files until you remember you need to open that GLB file
// TODO: Read 'success'
bool File::read(std::vector<uint8_t>& out_data) {
  char cursor;
  while (handle.get(cursor)) {
    out_data.push_back(cursor);
  }
    return true;
  }

std::string File::get_path() const {
  std::string path;
  switch (this->resource_type) {
    case PathType::RESOURCE:
      path = Filesystem::get_exec_path() + "/" + this->base_path;
      break;
      // no need to change. caller gave what was requested
    case PathType::ABSOLUTE:
    case PathType::USER:
      path = this->base_path;
      break;
  }
  return path;
}

