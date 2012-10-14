#include "solids.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <ostream>
#include <tuple>

using namespace oxatrace;

// So that tuples can be pretty-printed in test failures.
namespace {
  template <std::size_t I, std::size_t End, typename... Ts>
  struct pretty_print {
    static auto do_(std::ostream& out, std::tuple<Ts...> const& ts) 
      -> std::ostream& {
      if (I > 0 && End != 1) out << ", ";
      out << std::get<I>(ts);
      return pretty_print<I + 1, End, Ts...>::do_(out, ts);
    }
  };

  template <std::size_t End, typename... Ts>
  struct pretty_print<End, End, Ts...> {
    static auto do_(std::ostream& out, std::tuple<Ts...> const&) 
      -> std::ostream& {
      return out; 
    } 
  };
}

template <typename... Ts>
auto operator << (std::ostream& out,
                  std::tuple<Ts...> const& t) -> std::ostream& {
  return pretty_print<0, sizeof...(Ts), Ts...>::do_(out, t);
}

auto operator << (std::ostream& out, sphere const& s) -> std::ostream& {
  return out << "sphere{center = " << s.center() 
             << ", radius = " << s.radius() << "}";
}

auto operator << (std::ostream& out, plane const& p) -> std::ostream& {
  return out << "plane{point = " << p.point()
             << ", normal = " << p.normal().get() << "}";
}

auto operator << (std::ostream& out, ray const& r) -> std::ostream& {
  return out << "ray{origin = " << r.origin()
             << ", direction = " << r.direction().get() << "}";
}

struct sphere_intersection_test 
  : testing::TestWithParam<std::tuple<sphere, ray, unsigned>> { };

struct plane_intersection_test
  : testing::TestWithParam<std::tuple<plane, ray, unsigned>> { };

void normal_test(sphere const& sphere, ray const& ray, double param) {
  unit<vector3> const normal{sphere.normal_at(ray, param)};
  vector3 const       point {point_at(ray, param)};

  // Does the ray originate inside the sphere? If so, we expect the normal to
  // point inwards. Otherwise it is expected to point outwards.

  double const origin_center_dist = norm(sphere.center() - ray.origin());
  if (origin_center_dist >= sphere.radius())
    EXPECT_EQ(normal.get(), unit<vector3>{point - sphere.center()}.get());
  else
    EXPECT_EQ(normal.get(), unit<vector3>{-(point - sphere.center())}.get());
}

void normal_test(plane const& plane, ray const& ray, double param) {
  unit<vector3> const normal{plane.normal_at(ray, param)};
  vector3 const       point {point_at(ray, param)};

  // We want the normal to point into the half-space the ray originated in.
  // In other words, that the angle between (ray.origin - plane.point()) and
  // normal be less than 90 degrees. In other words cos alpha has to be 
  // in (0, 1].

  unit<vector3> const dir{ray.origin() - plane.point()};
  double const cos_alpha = dot(dir.get(), normal.get());
  EXPECT_LE(cos_alpha, 1) << dir.get() << " versus " << normal.get();
  EXPECT_GT(cos_alpha, 0) << dir.get() << " versus " << normal.get();
}

template <typename Solid>
void solid_intersection_test(Solid const& solid, ray const& ray,
                             unsigned expected_hits) {
  shape::intersection_list const intersections{solid.intersect(ray)};
  EXPECT_EQ(expected_hits, intersections.size());

  for (double param : intersections) {
    SCOPED_TRACE(std::string{"testing normal for param "} + 
                 std::to_string(param));
    normal_test(solid, ray, param);
  }
}

TEST_P(sphere_intersection_test, intersect_test) {
  sphere const&  sphere        = std::get<0>(GetParam());
  ray const      ray           = std::get<1>(GetParam());
  unsigned const expected_hits = std::get<2>(GetParam());

  solid_intersection_test(sphere, ray, expected_hits);
}

TEST_P(plane_intersection_test, intersect_test) {
  plane const&   plane         = std::get<0>(GetParam());
  ray const      ray           = std::get<1>(GetParam());
  unsigned const expected_hits = std::get<2>(GetParam());

  solid_intersection_test(plane, ray, expected_hits);
}

INSTANTIATE_TEST_CASE_P(
  sphere_tests,
  sphere_intersection_test,
  testing::Values(
    std::make_tuple(sphere{vector3{0.0, 0.0, 0.0}, 1.0},
                    ray{vector3{2.0, -3.0, 2.0}, vector3{-2.0, 3.0, -2.0}},
                    2),
    std::make_tuple(sphere{vector3{0.0, 0.0, 0.0}, 1.0},
                    ray{vector3{1.0/2.0, 1.0/2.0, 1.0/2.0},
                        vector3{1.0, 1.0, 1.0}},
                    1),
    std::make_tuple(sphere{vector3{0.0, 0.0, 0.0}, 1.0},
                    ray{vector3{3.0, 2.0, 5.0}, vector3{10.0, -1.0, 4.0}},
                    0),
    std::make_tuple(sphere{vector3{0.0, 0.0, 0.0}, 1.0},
                    ray{vector3{3.0, 2.0, 5.0}, vector3{-10.0, 1.0, -4.0}},
                    0),
    std::make_tuple(sphere{vector3{0.0, 0.0, 0.0}, 1.0},
                    ray{vector3{-3.0/2.0, -3.0/2.0, 5.0*std::sqrt(2.0)/2.0},
                        vector3{1.0, 1.0, -std::sqrt(2.0)}},
                    1),

    std::make_tuple(sphere{vector3{3.0, 2.0, 1.0}, 5.0},
                    ray{vector3{-4.0, 1.0, -8.0}, vector3{7.0, 1.0, 8.0}},
                    2),
    std::make_tuple(sphere{vector3{3.0, 2.0, 1.0}, 5.0},
                    ray{vector3{4.0, 3.0, -1.0}, vector3{2.0, -1.0, 8.0}},
                    1),
    std::make_tuple(sphere{vector3{3.0, 2.0, 1.0}, 5.0},
                    ray{vector3{-4.0, 1.0, -8.0}, vector3{15.0, -20.0, 3.0}},
                    0),
    std::make_tuple(sphere{vector3{3.0, 2.0, 1.0}, 5.0},
                    ray{vector3{-4.0, 1.0, -8.0}, vector3{-15.0, 20.0, -3.0}},
                    0),
    std::make_tuple(sphere{vector3{3.0, 2.0, 1.0}, 5.0},
                    ray{vector3{8.0, 2.0, -10.0}, vector3{0.0, 0.0, 1.0}},
                    1)
  )
);

INSTANTIATE_TEST_CASE_P(
  plane_tests,
  plane_intersection_test,
  testing::Values(
    std::make_tuple(plane{vector3{0.0, 0.0, 0.0}, vector3::unit_y()},
                    ray{vector3{0.0, 2.0, 0.0}, vector3{0.0, -1.0, 0.0}},
                    1),
    std::make_tuple(plane{vector3{0.0, 0.0, 0.0}, vector3::unit_y()},
                    ray{vector3{0.0, 2.0, 0.0}, vector3{1.0, -1.0, 1.0}},
                    1),
    std::make_tuple(plane{vector3{0.0, 0.0, 0.0}, vector3::unit_y()},
                    ray{vector3{0.0, -1.0, 2.0}, vector3{1.0, 1.0, 0.0}},
                    1),
    std::make_tuple(plane{vector3{0.0, 0.0, 0.0}, vector3::unit_y()},
                    ray{vector3{1.0, 1.0, 1.0}, vector3{1.0, 0.0, 0.0}},
                    0),
    std::make_tuple(plane{vector3{0.0, -2.0, 0.0}, vector3{0.0, -1.0, 0.0}},
                    ray{vector3{-1.0, 3.0, 0.0}, vector3{-10.0, -2.0, -9.0}},
                    1)
    )
);

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

