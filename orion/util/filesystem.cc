#ifndef ORION_UTIL_FILESYSTEM_CC_
#define ORION_UTIL_FILESYSTEM_CC_

#include "orion/util/filesystem.h"

#include <fstream>
#include <iostream>

File File::resource(const std::string& filename) {
  std::cout << "Opening resource file res://" << filename << "\n";
  File file;
  std::ofstream ofile;
  ofile.open(filename);
  if (ofile.is_open()) {
    std::cout << "Found file " << "res://" << filename << " successfully"
              << "\n";
  } else {
    std::cout << "File did not open" << "\n";
  }

  return file;
};

#endif  // ORION_UTIL_FILESYSTEM_CC_
