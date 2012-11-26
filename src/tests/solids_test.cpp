#include "solids.hpp"

#include <gtest/gtest.h>

#include <cassert>
#include <cmath>
#include <ostream>
#include <random>

using namespace oxatrace;

constexpr std::size_t ITERATIONS{100};

auto operator << (std::ostream& out, vector3 const& v) -> std::ostream& {
  return Eigen::operator << (out, v);
}

auto operator << (std::ostream& out, unit3 const& v) -> std::ostream& {
  return Eigen::operator << (out, v);
}

struct sphere_test : testing::Test {
  oxatrace::sphere                 sphere;
  
  std::default_random_engine       engine;
  std::uniform_real_distribution<> circle_distrib{0.0, 2.0 * PI};
  std::uniform_real_distribution<> half_circle_distrib{-PI / 2, PI / 2};
  std::uniform_real_distribution<> length_distrib{0.0, 10.};
  
  void expect_two_intersections(ray const& ray) {
    shape::intersection_list const intersections = sphere.intersect(ray);
  
    ASSERT_EQ(2u, intersections.size());

    double const entry{intersections.front()};
    double const exit {intersections.back()};

    ray_point entry_point{ray, entry};
    ray_point exit_point{ray, exit};

    expect_on_sphere(entry_point.point());
    expect_on_sphere(exit_point.point());

    expect_on_ray(ray, entry_point.point());
    expect_on_ray(ray, exit_point.point());

    unit3 const entry_normal{sphere.normal_at(entry_point)};
    unit3 const exit_normal {sphere.normal_at(exit_point)};

    EXPECT_TRUE(entry_point.point().isApprox(entry_normal, EPSILON))
      << "entry_point =\n" << entry_point.point()
      << "\nentry_normal =\n" << entry_normal;
    EXPECT_TRUE(exit_point.point().isApprox(exit_normal, EPSILON))
      << "exit_point =\b" << exit_point.point()
      << "\nexit_normal =\n" << exit_normal;
  }

  void expect_one_intersection(ray const& ray) {
    shape::intersection_list const intersections = sphere.intersect(ray);
    ASSERT_EQ(1u, intersections.size());

    ray_point tangent_point{ray, intersections.front()};

    expect_on_sphere(tangent_point.point());
    expect_on_ray(ray, tangent_point.point());

    unit3 const normal{sphere.normal_at(tangent_point)};
    EXPECT_TRUE(tangent_point.point().isApprox(normal, EPSILON))
      << "tangent_point =\n" << tangent_point.point()
      << "\nnormal =\n" << normal;
  }
    
  auto carth_from_polar(double r, double theta, double phi) -> vector3 {
    return r * vector3{
      std::sin(theta) * std::cos(phi),
      std::sin(theta) * std::sin(phi),
      std::cos(theta)
    };
  }

  void expect_on_sphere(vector3 const& point) {
    SCOPED_TRACE(point);
    EXPECT_NEAR(1.0, point.norm(), EPSILON);
  }

  void expect_on_ray(ray const& ray, vector3 const& point) {
    SCOPED_TRACE(point);

    vector3 const& ori = ray.origin();
    vector3 const& dir = ray.direction();
      
    double t_1{-1}, t_2{-1}, t_3{-1};
    if (double_neq(dir.x(), 0.0)) t_1 = (point.x() - ori.x()) / dir.x();
    if (double_neq(dir.y(), 0.0)) t_2 = (point.y() - ori.y()) / dir.y();
    if (double_neq(dir.z(), 0.0)) t_3 = (point.z() - ori.z()) / dir.z();

    EXPECT_TRUE(t_1 == -1.0 || t_1 > EPSILON);
    EXPECT_TRUE(t_2 == -1.0 || t_2 > EPSILON);
    EXPECT_TRUE(t_3 == -1.0 || t_3 > EPSILON);

    EXPECT_TRUE(t_1 == -1.0 || t_2 == -1.0 || double_eq(t_1, t_2));
    EXPECT_TRUE(t_1 == -1.0 || t_3 == -1.0 || double_eq(t_1, t_3));
    EXPECT_TRUE(t_2 == -1.0 || t_3 == -1.0 || double_eq(t_2, t_3));
  }
};

TEST_F(sphere_test, hit_test) {
  for (std::size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
    SCOPED_TRACE(iteration);
    
    // Get a point (1, theta, phi) on the sphere.
    double const theta{circle_distrib(engine)};
    double const phi  {half_circle_distrib(engine)};

    // Get a random vector pointing outwards from the sphere when originating
    // in the point (1, theta, phi).
    double const v_theta{theta + half_circle_distrib(engine)};
    double const v_phi  {phi + half_circle_distrib(engine)};

    // Convert (1, theta, phi) to carthesian coordinates to get the origin of
    // our ray, get a random distance and translate the origin by our random
    // vector (distance, v_theta, v_phi). Then set direction so that the ray
    // hits (1, theta, phi) and shoot it against the sphere.

    double const distance     {length_distrib(engine)};
    vector3 const point       {carth_from_polar(1, theta, phi)};
    vector3 const second_point{carth_from_polar(1, theta + PI, phi + PI)};
    vector3 const origin
      {point + distance * carth_from_polar(distance, v_theta, v_phi)};
    unit3 const dir{point - origin};

    assert(double_eq(point.norm(), 1.0));
    assert(double_eq(second_point.norm(), 1.0));

    expect_two_intersections({origin, dir});
  }
}

struct plane_test : testing::Test {
  oxatrace::plane                  plane;
  std::default_random_engine       engine;
  std::uniform_real_distribution<> length_distrib{0.0, 10.0};
  std::uniform_real_distribution<> space_distrib{-100.0, 100.0};
  std::uniform_int_distribution<>  sign_distrib{0, 1};
};

TEST_F(plane_test, hit_test) {
  for (std::size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
    // Get a random point on the xy plane then offset it by a random vector and
    // shoot ray that goes through the random point.

    auto sign = [&] { return sign_distrib(engine) ? +1 : -1; };

    vector3 const point{space_distrib(engine), space_distrib(engine), 0.0};
    unit3 const   dir  {length_distrib(engine),
                        length_distrib(engine),
                        sign() * (1.0 + length_distrib(engine))};

    ray const ray{point + length_distrib(engine) * dir, -dir};

    SCOPED_TRACE(ray);

    shape::intersection_list const intersections{plane.intersect(ray)};
    ASSERT_EQ(1u, intersections.size());

    ray_point const i_point{ray, intersections.front()};
    EXPECT_TRUE(i_point.point().isApprox(point, EPSILON));

    unit3 const normal{plane.normal_at(i_point)};
    double const cos_alpha{cos_angle(point - ray.origin(), normal)};

    EXPECT_GE(cos_alpha, 0);
    EXPECT_LE(cos_alpha, 1);
  }
}

TEST_F(plane_test, miss_test) {
  for (std::size_t iteration = 0; iteration < ITERATIONS; ++iteration) {
    // Get a random point not on the plane. If the point is in front of the
    // plane (positive z component), point it in a direction with a positive
    // z component, too. And vice-versa for point behind the plane.

    vector3 const point{space_distrib(engine),
                        space_distrib(engine),
                        space_distrib(engine)};
    unit3 const dir{
      length_distrib(engine),
      length_distrib(engine),
      (point.z() > 0.0 ? +1 : -1) * (1.0 + length_distrib(engine))
    };
    ray const ray{point, dir};

    SCOPED_TRACE(ray);

    shape::intersection_list const intersections{plane.intersect(ray)};
    EXPECT_TRUE(intersections.empty());
  }
}

struct solid_test : testing::Test {
  solid s{std::make_shared<sphere>(), material{{0, 0, 0}, 0.0, 0.0, 1}};

  void expect_hit(ray const& ray) {
    SCOPED_TRACE(ray);
    shape::intersection_list const intersections{s.intersect(ray)};
    EXPECT_TRUE(!intersections.empty());
  }

  void expect_hit(ray const& ray, vector3 center, double radius) {
    SCOPED_TRACE(ray);
    shape::intersection_list const intersections{s.intersect(ray)};
    EXPECT_TRUE(!intersections.empty());

    for (double i : intersections)
      EXPECT_NEAR((point_at(ray, i) - center).norm(), radius, EPSILON);
  }

  void expect_miss(ray const& ray) {
    SCOPED_TRACE(ray);
    shape::intersection_list const intersections{s.intersect(ray)};
    EXPECT_TRUE(intersections.empty());
  }
};

TEST_F(solid_test, translation_test) {
  SCOPED_TRACE("translation_test");
  s.translate({1.0, 2.0, 3.0});
  expect_hit({{1.0, 2.0, -5.0}, {0.0, 0.0, 1.0}},
             {1.0, 2.0, 3.0}, 1.0);
  expect_miss({{0.0, 0.0, 0.0}, {0.0, 0.0, 1.0}});
}

TEST_F(solid_test, uniform_scale_test) {
  SCOPED_TRACE("uniform_scale_test");
  ray r{{1.25, 1.25, -5.0}, {0.0, 0.0, 1.0}};
  
  expect_miss(r);
  s.scale(2.0);
  expect_hit(r, {0, 0, 0}, 2.0);
}

TEST_F(solid_test, anisotropic_scale_test) {
  SCOPED_TRACE("anisotropic_scale_test");
  ray r{{1.25, 0.5, -5.0}, {0.0, 0.0, 1.0}};

  expect_miss(r);
  s.scale(2.0, 1.0, 1.0);
  expect_hit(r);
}

auto main(int argc, char** argv) -> int {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
