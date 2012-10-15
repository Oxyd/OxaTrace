#include "math.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <type_traits>

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
  EXPECT_EQ("(1, 2, 3)", ss.str());
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

TEST(unit, unit_construction_assignment) {
  vector3 v{1.0, 2.0, -9.0};
  unit<vector3> unit_v{v};

  v = vector3{2.0, 5.0, -1.0};
  unit_v = v;
  
  unit<vector3> unit_u{unit_v};
  unit_v = unit_u;
}

TEST(unit, unit_contains_unit_vectors) {
  vector3 v{1.0, 2.0, -9.0};
  unit<vector3> uv{v};
  EXPECT_EQ(normalize(v), uv.get());
}

TEST(unit, unit_does_not_like_zero_vectors) {
  vector3 z{0.0, 0.0, 0.0};
  ASSERT_THROW(unit<vector3>{z}, std::logic_error);
}

template <typename U, typename V, typename W>
void test_unit_vector_addition(U const& u, V const& v, W const& result) {
  auto w = u + v;
  static_assert(std::is_same<decltype(w), W>::value,
                "Wrong addition result type");
  EXPECT_EQ(result, w);
}

template <typename U, typename V, typename W>
void test_unit_vector_subtraction(U const& u, V const& v, W const& result) {
  auto w = u - v;
  static_assert(std::is_same<decltype(w), W>::value,
                "Wrong subtraction result type");
  EXPECT_EQ(result, w);
}

TEST(unit, unit_vector_addition) {
  test_unit_vector_addition(unit<vector3>{1.0, 0.0, 0.0},
                            vector3{0.0, 1.0, 0.0},
                            vector3{1.0, 1.0, 0.0});
}

TEST(unit, vector_unit_addition) {
  test_unit_vector_addition(vector3{1.0, 0.0, 0.0},
                            unit<vector3>{0.0, 1.0, 0.0},
                            vector3{1.0, 1.0, 0.0});
}

TEST(unit, unit_unit_addition) {
  test_unit_vector_addition(unit<vector3>{1.0, 0.0, 0.0},
                            unit<vector3>{0.0, 1.0, 0.0},
                            vector3{1.0, 1.0, 0.0});
}

TEST(unit, unit_vector_subtraction) {
  test_unit_vector_subtraction(unit<vector3>{1.0, 0.0, 0.0},
                               vector3{0.0, 1.0, 0.0},
                               vector3{1.0, -1.0, 0.0});
}

TEST(unit, vector_unit_subtraction) {
  test_unit_vector_subtraction(vector3{1.0, 0.0, 0.0},
                               unit<vector3>{0.0, 1.0, 0.0},
                               vector3{1.0, -1.0, 0.0});
}

TEST(unit, unit_unit_subtraction) {
  test_unit_vector_subtraction(unit<vector3>{1.0, 0.0, 0.0},
                               unit<vector3>{0.0, 1.0, 0.0},
                               vector3{1.0, -1.0, 0.0});
}

TEST(unit, unit_scalar_multiplication) {
  unit<vector3> v{1.0, 0.0, 0.0};
  double scalar = 5.0;

  auto u = scalar * v;
  auto w = v * scalar;

  static_assert(std::is_same<decltype(u), vector3>::value,
                "Wrong result of scalar-unit multiplication");
  static_assert(std::is_same<decltype(w), vector3>::value,
                "Wrong result of unit-scalar multiplication");

  vector3 result{5.0, 0.0, 0.0};
  EXPECT_EQ(result, u);
  EXPECT_EQ(result, w);
}

TEST(unit, unit_scalar_division) {
  unit<vector3> v{1.0, 0.0, 0.0};
  double scalar = 2.0;

  auto u = v / scalar;

  static_assert(std::is_same<decltype(u), vector3>::value,
                "Wrong result of unit-scalar division.");

  vector3 result{0.5, 0.0, 0.0};
  EXPECT_EQ(result, u);
}

TEST(unit, unit_dot_product) {
  unit<vector3> a{1.0, 0.0, 0.0};
  unit<vector3> b{0.0, 1.0, 0.0};
  vector3       c{1.0, 1.0, 0.0};

  double x = dot(a, c);
  double y = dot(c, a);
  EXPECT_EQ(1.0, x);
  EXPECT_EQ(1.0, y);

  x = dot(a, b);
  y = dot(b, a);
  EXPECT_EQ(0.0, x);
  EXPECT_EQ(0.0, y);
}

TEST(unit, unit_cross_product) {
  unit<vector3> a{1.0, 0.0, 0.0};
  unit<vector3> b{0.0, 1.0, 0.0};
  vector3       c{1.0, 1.0, 0.0};

  auto x = cross(a, c);
  auto y = cross(c, a);

  static_assert(std::is_same<decltype(x), vector3>::value,
                "Wrong result of unit-vector cross product");
  static_assert(std::is_same<decltype(y), vector3>::value,
                "Wrong result of vector-unit cross product");

  EXPECT_EQ((vector3{0, 0, 1}), x);
  EXPECT_EQ((vector3{0, 0, -1}), y);

  x = cross(a, b);
  y = cross(b, a);

  EXPECT_EQ((vector3{0, 0, 1}), x);
  EXPECT_EQ((vector3{0, 0, -1}), y);
}

TEST(unit, unit_norm) {
  unit<vector3> x{1.0, 2.0, 3.0};
  EXPECT_EQ(1.0, norm(x));
  EXPECT_EQ(1.0, norm_squared(x));
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

