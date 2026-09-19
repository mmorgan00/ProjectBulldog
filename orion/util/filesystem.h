#ifndef ORION_UTIL_FILESYSTEM_H_
#define ORION_UTIL_FILESYSTEM_H_

#include <vector>
class File {
 public:
  /**
   * Returns true if file exists, false if not. Does not check if program is
   * able (permissions) to open file
   **/
  static bool exists(std::string filename);
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

#endif  // ORION_UTIL_FILESYSTEM_H_
