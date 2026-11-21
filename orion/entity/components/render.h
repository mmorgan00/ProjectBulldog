#ifndef ORION_ENTITY_COMPONENTS_RENDER_H_
#define ORION_ENTITY_COMPONENTS_RENDER_H_

#include <entt/entt.hpp>

#include "orion/core/renderer_types.h"
#include "orion/util/logger.h"

DECLARE_LOG_CATEGORY(RENDER_CMPNT);

struct RenderComponent {
  RenderObject* handle{};
};

#endif  // ORION_ENTITY_COMPONENTS_RENDER_H_
