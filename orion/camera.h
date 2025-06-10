// Copyright 2025 Max Morgan
#ifndef ORION_CAMERA_H_
#define ORION_CAMERA_H_

#include <SDL_events.h>
#include <engine_types.h>

class Camera {
 public:
  glm::vec3 velocity;
  glm::vec3 position;
  // vertical rotation
  float pitch{0.f};
  // horizontal rotation
  float yaw{0.f};
  glm::mat4 getViewMatrix();
  glm::mat4 getRotationMatrix();

  void processSDLEvent(SDL_Event& e);

  void update();
};

#endif  // ORION_CAMERA_H_
