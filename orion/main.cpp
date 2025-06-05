
#include "simdjson.h"

#include "entry.hpp"
#include "core/renderer.hpp"
#include "util/logger.hpp"

using namespace simdjson;

int main(int argc, char* argv[]) {
  DECLARE_LOG_CATEGORY(ORION);
  // Load config
  ondemand::parser parser;
  padded_string json = padded_string::load("../../config/engine.conf");
  ondemand::document config = parser.iterate(json);
  OE_LOG(ORION, INFO, "{}", std::string(config["appName"]));
  OE_LOG(ORION, INFO, "Running using {}", std::string(config["graphicsAPI"]));
  // Init modules
  Renderer renderer;
  renderer.init();
  // Run game
  init();
  run();
  // Cleanup process
  // TODO: Expose a 'quit game' api
  renderer.shutdown();
  return 0;
}
