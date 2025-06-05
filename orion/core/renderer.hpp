#pragma once
#include "renderer_backend.hpp"


class Renderer{
public:

void init();
void shutdown();
private:

  RendererBackend* backend;
};
