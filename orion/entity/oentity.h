// Copyright 2025 Max Morgan
#ifndef ORION_ENTITY_OENTITY_H_
#define ORION_ENTITY_OENTITY_H_

#include <SDL.h>

class OEntity {
 public:
  virtual void update() = 0;
  virtual void handleInputEvent(SDL_Event& e) = 0;

  virtual ~OEntity() {}
};

#endif  // ORION_ENTITY_OENTITY_H_
