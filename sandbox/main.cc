// // Copyright 2025 Max Morgan

#include <string>
#include <thread>

#include "SDL_events.h"
#include "SDL_video.h"
#include "orion/asset/primitives.h"
#include "orion/core/asset_registry.h"
#include "orion/core/engine_types.h"
#include "orion/core/renderer.h"
#include "orion/core/resource_types/static_mesh.h"
#include "orion/entity/camera.h"
#include "orion/entry.h"
#include "orion/util/logger.h"

int main(int argc, char *argv[]) {
  DECLARE_LOG_CATEGORY(ORION);
  AppState state;
  // Load config
  simdjson::ondemand::parser parser;
  simdjson::padded_string json =

      simdjson::padded_string::load("../../config/engine.conf");
  simdjson::ondemand::document config = parser.iterate(json);
  std::string_view graphicsAPI_sv = config["graphicsAPI"].get_string();
  std::string_view entry_scene_sv = config["entryScene"].get_string();
  std::string graphicsAPI = std::string(graphicsAPI_sv);
  std::string entry_scene = std::string(entry_scene_sv);

  state.build(config);

  Camera mainCamera;

  mainCamera.velocity = glm::vec3(0.f);
  mainCamera.position = glm::vec3(30.f, -00.f, -085.f);

  mainCamera.pitch = 0;
  mainCamera.yaw = 0;

  OE_LOG(ORION, INFO, "{}", state.appName);
  OE_LOG(ORION, INFO, "Running using {}", graphicsAPI);
  // Init modules
  Renderer renderer;
  renderer.init(state);
  renderer.set_camera(&mainCamera);
  // Call game initialization
  init();

  OE_LOG(ORION, INFO, "Loading initial scene {}", entry_scene);
  // renderer.loadScene(entry_scene);
  AssetRegistry<StaticMesh> staticMeshRegistry;
  Primitive cube = primitives::cube();
  StaticMesh cube_sm = StaticMesh{
      .name = "Test", .vertices = cube.vertices, .indices = cube.indices};
  Resource<StaticMesh> handle = staticMeshRegistry.insert(cube_sm);

  OE_LOG(ORION, DEBUG, "Static Mesh registry size {}",
         staticMeshRegistry.size());
  OE_LOG(ORION, DEBUG, "Static Mesh registry entry generation {}",
         handle.generation);
  OE_LOG(ORION, DEBUG, "Static Mesh registry entry retrieval name {}",
         staticMeshRegistry.get(handle)->name);
  renderer.loadStaticMesh(staticMeshRegistry.get(handle));

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
