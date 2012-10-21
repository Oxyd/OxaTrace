#include "math.hpp"

#include <gtest/gtest.h>

using namespace oxatrace;

TEST(unit_test, unit_creation) {
  Eigen::Vector3d v;
  v << 1, 2, 3;
  unit<Eigen::Vector3d> u(v);
  EXPECT_EQ(1.0, u.norm());
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

