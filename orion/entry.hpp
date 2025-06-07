#pragma once

class IGameState{
  virtual void init() = 0;
  virtual void handleEvents() = 0;
  virtual void exit() = 0;
};

extern void init();



