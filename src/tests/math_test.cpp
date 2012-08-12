#include "math.hpp"
#include <gtest/gtest.h>

using namespace oxatrace;

TEST(vector, unit_construction_assignment) {
  vector3 v{1.0, 2.0, -9.0};
  unit<vector3> unit_v{v};

  v = vector3{2.0, 5.0, -1.0};
  unit_v = v;
  
  unit<vector3> unit_u{unit_v};
  unit_v = unit_u;
}

TEST(vector, unit_contains_unit_vectors) {
  vector3 v{1.0, 2.0, -9.0};
  unit<vector3> uv{v};
  EXPECT_EQ(v.normalized(), uv.get());
}

TEST(vector, unit_does_not_like_zero_vectors) {
  vector3 z{0.0, 0.0, 0.0};
  ASSERT_THROW(unit<vector3>{z}, std::logic_error);
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

