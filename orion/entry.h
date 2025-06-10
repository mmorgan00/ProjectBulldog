// Copyright 2025 Max Morgan
#ifndef ORION_ENTRY_H_
#define ORION_ENTRY_H_
#pragma once

class IGameState {
  virtual void init() = 0;
  virtual void handleEvents() = 0;
  virtual void exit() = 0;
};

extern void init();

#endif  // ORION_ENTRY_H_
