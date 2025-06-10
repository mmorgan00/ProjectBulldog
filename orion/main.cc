#include "simdjson.h"
#include <SDL.h>

#include "core/engine_types.h"
#include "core/renderer.h"
#include "entry.h"
#include "util/logger.h"

using namespace simdjson;

int main(int argc, char *argv[]) {
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
  // Call game initialization
  init();

  // Main loop
  bool bQuit = false;
  SDL_Event e;
  while (!bQuit) {
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (e.type == SDL_QUIT)
        bQuit = true;

      // Handle keypress
      if (e.type == SDL_KEYDOWN) {
        // Another way to quit
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          OE_LOG(ORION, INFO, "Quitting...");
          bQuit = true;
        }
      }
      // MAIN GAME LOOP
      // TODO: Proper tick loop -> should be endless loop until quit signal
      // recevied
    }
    renderer.draw();
  }

  // Cleanup process
  // TODO: Expose a 'quit game' api
  OE_LOG(ORION, INFO, "Calling renderer cleanup");
  renderer.cleanup();
  return 0;
}
