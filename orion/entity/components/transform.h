#ifndef ORION_ENTITY_COMPONENTS_TRANSFORM_H_
#define ORION_ENTITY_COMPONENTS_TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include "orion/core/scripting.h"
#include "orion/util/logger.h"

DECLARE_LOG_CATEGORY(TRANSFORM_CMPNT);
class TransformComponent {
 public:
  glm::vec3 position{0.0F};
  glm::vec3 rotation{0.0F};
  glm::vec3 scale{1.0F};

  glm::mat4 getMatrix() const {
    return glm::translate(glm::mat4(1.0F), position) *
           glm::mat4_cast(glm::quat(rotation)) *
           glm::scale(glm::mat4(1.0F), scale);
  }

  Callable set_rotation{[](const Args& args) -> Value {
    OE_LOG(TRANSFORM_CMPNT, INFO, "SET ROTATION: {} {} {}", args[0].number,
           args[1].number, args[2].number);
    return Value{.type = Value::Type::Null};
  }};
};

#endif  // ORION_ENTITY_COMPONENTS_TRANSFORM_H_
