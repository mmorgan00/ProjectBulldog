#include "orion/entity/components/transform.h"

#include <gtest/gtest.h>

const float ERR_MARGIN = 1e-5F;

static void expectMatNear(const glm::mat4& lhs, const glm::mat4& rhs,
                          float eps = ERR_MARGIN) {
  for (int col = 0; col < 4; ++col)
    for (int row = 0; row < 4; ++row)
      EXPECT_NEAR(lhs[col][row], rhs[col][row], eps)
          << "at col " << col << " row " << row;
}

TEST(TransformComponent, IdentityGetMatrixGeneratesIdentity) {
  TransformComponent identity;
  EXPECT_EQ(glm::mat4(1.0F), identity.getMatrix());
}

TEST(TransformComponent, SingleAxisRotationsMatchReference) {
  const float theta = glm::radians(30.0F);
  const float cos_t = std::cos(theta);
  const float sin_t = std::sin(theta);

  const glm::mat4 rotx(1, 0, 0, 0, 0, cos_t, sin_t, 0, 0, -sin_t, cos_t, 0, 0,
                       0, 0, 1);
  const glm::mat4 roty(cos_t, 0, -sin_t, 0, 0, 1, 0, 0, sin_t, 0, cos_t, 0, 0,
                       0, 0, 1);
  const glm::mat4 rotz(cos_t, sin_t, 0, 0, -sin_t, cos_t, 0, 0, 0, 0, 1, 0, 0,
                       0, 0, 1);

  TransformComponent tcmp;
  tcmp.rotation = {theta, 0, 0};
  expectMatNear(tcmp.getMatrix(), rotx);
  tcmp.rotation = {0, theta, 0};
  expectMatNear(tcmp.getMatrix(), roty);
  tcmp.rotation = {0, 0, theta};
  expectMatNear(tcmp.getMatrix(), rotz);
}
