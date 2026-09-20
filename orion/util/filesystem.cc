#ifndef ORION_UTIL_FILESYSTEM_CC_
#define ORION_UTIL_FILESYSTEM_CC_

#include "orion/util/filesystem.h"

#include <iostream>

File File::resource(const std::string& filename) {
  std::cout << "Opening resource file res://" << filename << "\n";
  File file;
  return file;
};

#endif  // ORION_UTIL_FILESYSTEM_CC_
