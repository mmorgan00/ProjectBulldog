#pragma once
#include "renderer_backend.hpp"


class Renderer{
public:

void init(app_state& state);
void shutdown();
void run();
private:

  RendererBackend* backend;
};
