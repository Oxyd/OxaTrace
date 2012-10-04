#include "math.hpp"
#include <gtest/gtest.h>
#include <sstream>

using namespace oxatrace;

struct vector_test : testing::Test {
  vector3 u{1.0, 2.0, 3.0};
  vector3 v{3.0, -2.0, -8.0};
  vector4 w{1.0, 2.0, 3.0, 4.0};
};

TEST_F(vector_test, equality) {
  vector3 a{1.0, 2.0, 3.0};
  EXPECT_EQ(a, u);
  EXPECT_EQ(u, a);

  EXPECT_NE(a, v);
  EXPECT_NE(v, a);
}

TEST_F(vector_test, member_access) {
  EXPECT_EQ(w.x(), 1.0);
  EXPECT_EQ(w.y(), 2.0);
  EXPECT_EQ(w.z(), 3.0);
  EXPECT_EQ(w.w(), 4.0);

  EXPECT_EQ(w[0], 1.0);
  EXPECT_EQ(w[1], 2.0);
  EXPECT_EQ(w[2], 3.0);
  EXPECT_EQ(w[3], 4.0);
}

TEST_F(vector_test, streaming) {
  std::ostringstream ss;
  ss << u;
  EXPECT_EQ("(1, 2, 3)^T", ss.str());
}

TEST_F(vector_test, addition) {
  vector3 a = u + v;
  vector3 b = v + u;
  vector3 c{4.0, 0.0, -5.0};

  EXPECT_EQ(c, a);
  EXPECT_EQ(c, b);
}

TEST_F(vector_test, subtraction) {
  vector3 a = u - v;
  vector3 b = v - u;
  vector3 c{-2.0, 4.0, 11.0};
  vector3 d{2.0, -4.0, -11.0};

  EXPECT_EQ(c, a);
  EXPECT_EQ(d, b);
}

TEST_F(vector_test, scalar_multiplication) {
  vector3 a = 3.0 * u;
  vector3 b = u * 3.0;
  vector3 c{3.0, 6.0, 9.0};

  EXPECT_EQ(c, a);
  EXPECT_EQ(c, b);
}

TEST_F(vector_test, scalar_division) {
  vector3 a = u / 3.0;
  EXPECT_EQ((vector3{1.0/3.0, 2.0/3.0, 1.0}), a);
}

TEST_F(vector_test, dot_product) {
  double a = dot(u, v);
  double b = dot(v, u);
  double c = -25.0;

  EXPECT_EQ(c, a);
  EXPECT_EQ(c, b);
}

TEST_F(vector_test, cross_product) {
  vector3 a = cross(u, v);
  vector3 b = cross(v, u);
  vector3 c{-10.0, 17.0, -8.0};
  vector3 d{10.0, -17.0, 8.0};

  EXPECT_EQ(c, a);
  EXPECT_EQ(d, b);
}

TEST_F(vector_test, norm) {
  double a = norm(u);
  double b = norm(v);

  EXPECT_DOUBLE_EQ(std::sqrt(14.0), a);
  EXPECT_DOUBLE_EQ(std::sqrt(77.0), b);
}

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
  EXPECT_EQ(normalize(v), uv.get());
}

TEST(vector, unit_does_not_like_zero_vectors) {
  vector3 z{0.0, 0.0, 0.0};
  ASSERT_THROW(unit<vector3>{z}, std::logic_error);
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

