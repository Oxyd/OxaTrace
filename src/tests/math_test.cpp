#include "math.hpp"

#include <gtest/gtest.h>

using namespace oxatrace;

TEST(unit_test, unit_creation) {
  Eigen::Vector3d v;
  v << 1, 2, 3;
  unit<Eigen::Vector3d> u(v);
  EXPECT_EQ(1.0, u.norm());
}

TEST(vector_test, get_any_orthogonal_test) {
  vector3 v{1, 2, 3};
  unit3   v_perp = get_any_orthogonal(v);
  EXPECT_NEAR(0.0, v.dot(v_perp), EPSILON);

  vector3 u{3, 2, 1};
  unit3   u_perp = get_any_orthogonal(u);
  EXPECT_NEAR(0.0, u.dot(u_perp), EPSILON);

  vector3 w{2, 3, 1};
  unit3   w_perp = get_any_orthogonal(w);
  EXPECT_NEAR(0.0, w.dot(w_perp), EPSILON);

  vector3 x{0, 0, 0};
  EXPECT_THROW(get_any_orthogonal(x), std::invalid_argument);
}

TEST(vector_test, reflection_test) {
  vector3 n{0.0, -1.0, 0.0};
  unit3   v{1.0, 1.0, 0.0};
  unit3   v_reflected = reflect(v, n);
  EXPECT_NEAR(0.0, (v_reflected - unit3{1.0, -1.0, 0.0}).norm(), EPSILON);
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

