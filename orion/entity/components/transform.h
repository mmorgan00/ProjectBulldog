#ifndef ORION_ENTITY_COMPONENTS_TRANSFORM_H_
#define ORION_ENTITY_COMPONENTS_TRANSFORM_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct TransformComponent {
  glm::vec3 position{0.0F};
  glm::vec3 rotation{0.0F};
  glm::vec3 scale{1.0F};

  glm::mat4 getMatrix() const {
    return glm::translate(glm::mat4(1.0F), position) *
           glm::mat4_cast(glm::quat(rotation)) *
           glm::scale(glm::mat4(1.0F), scale);
  }
};

#endif  // ORION_ENTITY_COMPONENTS_TRANSFORM_H_
