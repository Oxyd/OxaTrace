#include "solids.hpp"
#include <gtest/gtest.h>
#include <cmath>

using namespace oxatrace;

struct sphere_test : testing::Test {
  sphere s1{vector3{0.0, 0.0, 0.0}, 1.0},
         s2{vector3{3.0, 2.0, 1.0}, 5.0};

  ray s1_hit{vector3{2.0, -3.0, 2.0}, vector3{-2.0, 3.0, -2.0}},
      s1_inside_hit{vector3{1.0/2.0, 1.0/2.0, 1.0/2.0}, vector3{1.0, 1.0, 1.0}},
      s1_miss_1{vector3{3.0, 2.0, 5.0}, vector3{10.0, -1.0, 4.0}},
      s1_miss_2{s1_hit.origin(), unit<vector3>{-s1_hit.direction().get()}},
      s1_tangent{vector3{-3.0/2.0, -3.0/2.0, 5.0*std::sqrt(2.0)/2.0},
                 vector3{1.0, 1.0, -std::sqrt(2.0)}},

      s2_hit{vector3{-4.0, 1.0, -8.0}, vector3{7.0, 1.0, 8.0}},
      s2_inside_hit{vector3{4.0, 3.0, -1.0}, vector3{2.0, -1.0, 8.0}},
      s2_miss_1{vector3{-4.0, 1.0, -8.0}, vector3{15.0, -20.0, 3.0}},
      s2_miss_2{s2_hit.origin(), unit<vector3>{-s2_hit.direction().get()}},
      s2_tangent{vector3{5.0, 6.0, 1.0 + std::sqrt(5.0)},
                 vector3{0.0, 1.0, 1.0 + 7.0 * std::sqrt(5.0)}};
};

TEST_F(sphere_test, hit_test) {
  auto const s1_intersections = s1.intersect_both(s1_hit);
  EXPECT_FALSE(zero(std::get<0>(s1_intersections)));
  EXPECT_FALSE(zero(std::get<1>(s1_intersections)));

  auto const s2_intersections = s2.intersect_both(s2_hit);
  EXPECT_FALSE(zero(std::get<0>(s2_intersections)));
  EXPECT_FALSE(zero(std::get<1>(s2_intersections)));
}

TEST_F(sphere_test, inside_hit_test) {
  auto const s1_intersections = s1.intersect_both(s1_inside_hit);
  EXPECT_FALSE(zero(std::get<0>(s1_intersections)));
  EXPECT_TRUE(zero(std::get<1>(s1_intersections)));

  auto const s2_intersections = s2.intersect_both(s2_inside_hit);
  EXPECT_FALSE(zero(std::get<0>(s2_intersections)));
  EXPECT_TRUE(zero(std::get<1>(s2_intersections)));
}

TEST_F(sphere_test, miss_test) {
  auto const s1_intersections_1 = s1.intersect_both(s1_miss_1);
  EXPECT_TRUE(zero(std::get<0>(s1_intersections_1)));
  EXPECT_TRUE(zero(std::get<1>(s1_intersections_1)));

  auto const s1_intersections_2 = s1.intersect_both(s1_miss_2);
  EXPECT_TRUE(zero(std::get<0>(s1_intersections_2)));
  EXPECT_TRUE(zero(std::get<1>(s1_intersections_2)));

  auto const s2_intersections_1 = s2.intersect_both(s2_miss_1);
  EXPECT_TRUE(zero(std::get<0>(s2_intersections_1)));
  EXPECT_TRUE(zero(std::get<1>(s2_intersections_1)));

  auto const s2_intersections_2 = s2.intersect_both(s2_miss_2);
  EXPECT_TRUE(zero(std::get<0>(s2_intersections_2)));
  EXPECT_TRUE(zero(std::get<1>(s2_intersections_2)));
}

TEST_F(sphere_test, tangent_test) {
  auto const s1_intersections = s1.intersect_both(s1_tangent);
  EXPECT_FALSE(zero(std::get<0>(s1_intersections)));
  EXPECT_TRUE(zero(std::get<1>(s1_intersections)));

  auto const s2_intersections = s2.intersect_both(s2_tangent);
  EXPECT_FALSE(zero(std::get<0>(s2_intersections)));
  EXPECT_TRUE(zero(std::get<1>(s2_intersections)));
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
