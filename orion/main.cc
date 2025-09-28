// // Copyright 2025 Max Morgan

#include <string>
#include <thread>

#include "SDL_events.h"
#include "SDL_video.h"
#include "core/engine_types.h"
#include "core/renderer.h"
#include "orion/entry.h"
#include "orion/util/logger.h"

int main(int argc, char *argv[]) {
  DECLARE_LOG_CATEGORY(ORION);
  app_state state;
  // Load config
  simdjson::ondemand::parser parser;
  simdjson::padded_string json =
      simdjson::padded_string::load("../../config/engine.conf");
  simdjson::ondemand::document config = parser.iterate(json);

  state.build(config);

  Camera mainCamera;

  mainCamera.velocity = glm::vec3(0.f);
  mainCamera.position = glm::vec3(30.f, -00.f, -085.f);

  mainCamera.pitch = 0;
  mainCamera.yaw = 0;

  OE_LOG(ORION, INFO, "{}", std::string(state.appName));
  OE_LOG(ORION, INFO, "Running using {}", std::string(config["graphicsAPI"]));
  // Init modules
  Renderer renderer;
  renderer.init(state);
  renderer.set_camera(&mainCamera);
  // Call game initialization
  init();

  // renderer.loadScene("demo");

  // Main loop
  bool bQuit = false;
  bool bStopRunning = false;
  bool resize_requested = false;
  SDL_Event e;
  while (!bQuit) {
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (e.type == SDL_QUIT) bQuit = true;

      mainCamera.handleInputEvent(e);
      // Handle keypress
      if (e.type == SDL_KEYDOWN) {
        // Another way to quit
        if (e.key.keysym.sym == SDLK_ESCAPE) {
          OE_LOG(ORION, INFO, "Quitting...");
          bQuit = true;
        }
      }
      if (e.type == SDL_WINDOWEVENT) {
        if (e.window.event == SDL_WINDOWEVENT_MINIMIZED) {
          bStopRunning = true;
        }
        if (e.window.event == SDL_WINDOWEVENT_RESTORED) {
          bStopRunning = false;
        }
        if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
          resize_requested = true;
        }
      }

      if (bStopRunning) {
        // throttle the speed to avoid the endless spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        continue;
      }
      // MAIN GAME LOOP
      // TODO: Proper tick loop -> should be endless loop until quit signal
      // recevied
    }
    if (resize_requested) {
      renderer.resize_window();
      resize_requested = false;
      continue;  // skip the draw call this frame
    }

    renderer.draw();
  }

  // Cleanup process
  // TODO: Expose a 'quit game' api
  OE_LOG(ORION, INFO, "Calling renderer cleanup");
  renderer.cleanup();
  return 0;
}
