#ifndef ORION_UTIL_FILESYSTEM_H_
#define ORION_UTIL_FILESYSTEM_H_

#include <vector>

#include "orion/util/logger.h"

enum class PathType : std::uint8_t { RESOURCE, USER, ABSOLUTE };
#include <fstream>

DECLARE_LOG_CATEGORY(FILESYSTEM);

class File {
  std::fstream handle;

 public:
  /**
   * Opens an existing resource file. If the requested file does not exist,
   * returns false
   **/
  static bool resource(const std::string& filename, File* out_file);
  static File user(std::string filename);
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
};

class Filesystem {
 public:
  static bool rename(std::string filename_from, std::string filename_to);
  static bool remove(std::string to_remove);
  static bool mkdir(std::string path);
};

#endif  // ORION_UTIL_FILESYSTEM_H_
