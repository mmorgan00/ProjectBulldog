#ifndef ORION_UTIL_FILESYSTEM_H_
#define ORION_UTIL_FILESYSTEM_H_

#include <cstdint>
#include <vector>

#include "orion/util/logger.h"

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__linux)
#define PLATFORM_LINUX
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#endif

#include <fstream>

enum class PathType : std::uint8_t { RESOURCE, USER, ABSOLUTE };
#include <fstream>

DECLARE_LOG_CATEGORY(FILESYSTEM);

class File {
  std::fstream handle;

 public:
  // Fallback case
  File() : resource_type(PathType::ABSOLUTE) {}
  /**
   * Opens an existing resource file. If the requested file does not exist,
   * returns false
   **/
  static bool resource(const std::string& filename, File* out_file);
  static File user(std::string filename, File* out_file);
  /**
   * Returns true if able to read, false if not. Contents are returned in out
   * parameter
   * */
  bool read(std::vector<uint8_t>& out_data);
  /**
   * Writes contents to file. Replaces whatever content existed previously.
   **/
  bool write(std::vector<uint8_t>);
  /**
   * Appends contents to file. Returns true if successful, false if fail
   **/
  bool append(std::vector<uint8_t>);

  /**
   *Dervies filepath based on filetype.
   **/
  std::string get_path() const;

 private:
  PathType resource_type{PathType::ABSOLUTE};
  std::string base_path;
};

class Filesystem {
 public:
  static bool rename(std::string filename_from, std::string filename_to);
  static bool remove(std::string to_remove);
  static bool mkdir(std::string path);
  static std::string get_exec_path();
};

#endif  // ORION_UTIL_FILESYSTEM_H_
