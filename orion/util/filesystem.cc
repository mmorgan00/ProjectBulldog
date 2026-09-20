#ifndef ORION_UTIL_FILESYSTEM_CC_
#define ORION_UTIL_FILESYSTEM_CC_

#include "orion/util/filesystem.h"

#include <ios>
#include <iostream>

bool File::resource(const std::string& filename, File* out_file) {
  OE_LOG(FILESYSTEM, INFO, "Creating resource file handle res://{}", filename);
  out_file->handle.open(filename, std::ios::in | std::ios::out);
  if (out_file->handle.is_open()) {
    OE_LOG(FILESYSTEM, INFO, "Opened file res://{} ", filename);
    return true;
  }
  OE_LOG(FILESYSTEM, INFO, "Failed to open file res://{} ", filename);
  return false;
};

#endif  // ORION_UTIL_FILESYSTEM_CC_
