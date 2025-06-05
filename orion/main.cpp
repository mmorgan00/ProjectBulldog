
#include "simdjson.h"

#include "entry.hpp"
#include "core/renderer.hpp"
#include "core/engine_types.hpp"
#include "util/logger.hpp"

using namespace simdjson;

int main(int argc, char* argv[]) {
  DECLARE_LOG_CATEGORY(ORION);
  app_state state;
  // Load config
  ondemand::parser parser;
  padded_string json = padded_string::load("../../config/engine.conf");
  ondemand::document config = parser.iterate(json);
  state.build(config);
  OE_LOG(ORION, INFO, "{}", std::string(state.appName));
  OE_LOG(ORION, INFO, "Running using {}", std::string(config["graphicsAPI"]));
  // Init modules
  Renderer renderer;
  renderer.init(state);
  // Run game
  init();
  run();
  // Cleanup process
  // TODO: Expose a 'quit game' api
  renderer.shutdown();
  return 0;
}
