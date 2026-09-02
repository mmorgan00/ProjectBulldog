#ifndef ORION_ENTRY_CC_
#define ORION_ENTRY_CC_

#include "orion/entry.h"

#include "SDL_events.h"
#include "orion/core/renderer.h"
#include "orion/entity/camera.h"
#include "orion/util/logger.h"

int main(void) {
  DECLARE_LOG_CATEGORY(ENGINE);

  OE_LOG(ENGINE, INFO, "ORION STARTING");

  AppState state;
  // Load config
  simdjson::ondemand::parser parser;
  simdjson::padded_string json =

      simdjson::padded_string::load("../../config/engine.conf");
  simdjson::ondemand::document config = parser.iterate(json);
  std::string_view graphicsAPI_sv = config["graphicsAPI"].get_string();
  // std::string_view entry_scene_sv = config["entryScene"].get_string();
  std::string graphicsAPI = std::string(graphicsAPI_sv);
  // std::string entry_scene = std::string(entry_scene_sv);

  state.build(config);

  Camera mainCamera;

  mainCamera.velocity = glm::vec3(0.F);
  mainCamera.position = glm::vec3(00.F, 00.F, 05.F);

  mainCamera.pitch = 0;
  mainCamera.yaw = 0;

  OE_LOG(ENGINE, INFO, "{}", state.appName);
  OE_LOG(ENGINE, INFO, "Running using {}", graphicsAPI);
  // Init modules
  Renderer renderer;
  renderer.init(state);
  renderer.set_camera(&mainCamera);
  // Call game initialization
  OE_init();

  bool bQuit = false;
  // bool resize_requested = false;
  SDL_Event event;
  while (!bQuit) {
    OE_update();
    // Handle events on queue
    while (SDL_PollEvent(&event) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (event.type == SDL_QUIT) {
        bQuit = true;
      };

      // Handle keypress
      if (event.type == SDL_KEYDOWN) {
        // Another way to quit
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          OE_LOG(ENGINE, INFO, "Quitting...");
          bQuit = true;
        }
      }
    }
  }
  OE_shutdown();
  return 0;
}

#endif  // ORION_ENTRY_CC_
