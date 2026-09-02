#ifndef ORION_ENTRY_CC_
#define ORION_ENTRY_CC_

#include "orion/entry.h"

#include "SDL_events.h"
#include "orion/core/asset_registry.h"
#include "orion/core/renderer.h"
#include "orion/core/resource_types/static_mesh.h"
#include "orion/entity/camera.h"
#include "orion/util/logger.h"

RenderObject* init_test_cube(AssetRegistry<StaticMesh>* assreg,
                             Renderer* renderer) {
  DECLARE_LOG_CATEGORY(ENGINE_TEST);
  Primitive cube = primitives::cube();
  StaticMesh cube_sm = StaticMesh{
      .name = "Test", .vertices = cube.vertices, .indices = cube.indices};
  Resource<StaticMesh> handle = assreg->insert(cube_sm);

  OE_LOG(ENGINE_TEST, DEBUG, "Static Mesh registry size {}", assreg->size());
  OE_LOG(ENGINE_TEST, DEBUG, "Static Mesh registry entry generation {}",
         handle.generation);
  OE_LOG(ENGINE_TEST, DEBUG, "Static Mesh registry entry retrieval name {}",
         assreg->get(handle)->name);
  RenderObject* test_mesh = renderer->loadStaticMesh(assreg->get(handle));
  return test_mesh;
}

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
  mainCamera.position = glm::vec3(4.0F, 02.F, 05.F);

  mainCamera.pitch = -0.5F;
  mainCamera.yaw = -1.0F;

  OE_LOG(ENGINE, INFO, "{}", state.appName);
  OE_LOG(ENGINE, INFO, "Running using {}", graphicsAPI);
  // Init modules
  Renderer renderer;
  renderer.init(state);
  renderer.set_camera(&mainCamera);
  // Call game initialization
  OE_init();

  AssetRegistry<StaticMesh> staticMeshRegistry;
  RenderObject* default_cube = init_test_cube(&staticMeshRegistry, &renderer);

  engine::DrawContext dctx{.OpaqueSurfaces = {default_cube},
                           .TransparentSurfaces = {}};

  bool bQuit = false;
  // bool resize_requested = false;
  auto last_frame_time = std::chrono::high_resolution_clock::now();
  SDL_Event event;
  while (!bQuit) {
    const auto current_time = std::chrono::high_resolution_clock::now();
    const auto delta =
        std::chrono::duration<float>(current_time - last_frame_time);
    float delta_time = delta.count();  // seconds as a flot
    last_frame_time = current_time;
    OE_update(delta_time);
    // Handle events on queue
    while (SDL_PollEvent(&event) != 0) {
      // close the window when user alt-f4s or clicks the X button
      if (event.type == SDL_QUIT) {
        bQuit = true;
      };
      mainCamera.handleInputEvent(event);

      // Handle keypress
      if (event.type == SDL_KEYDOWN) {
        // Another way to quit
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          OE_LOG(ENGINE, INFO, "Quitting...");
          bQuit = true;
        }
      }
    }
    renderer.draw(dctx);
  }
  OE_shutdown();
  return 0;
}

#endif  // ORION_ENTRY_CC_
