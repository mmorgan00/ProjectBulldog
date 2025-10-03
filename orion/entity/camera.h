// Copyright 2025 Max Morgan

#ifndef ORION_ENTITY_CAMERA_H_
#define ORION_ENTITY_CAMERA_H_

#include <SDL.h>
#include <orion/entity/oentity.h>

#include <glm/vec3.hpp>

class Camera : OEntity {
 public:
  glm::vec3 velocity;
  glm::vec3 position;
  // vertical rotation
  float pitch{0.f};
  // horizontal rotation
  float yaw{0.f};

  glm::mat4 getViewMatrix();
  glm::mat4 getRotationMatrix();

  void handleInputEvent(SDL_Event& e) override;
  void update() override;

  ~Camera() {}
};

#endif  // ORION_ENTITY_CAMERA_H_
