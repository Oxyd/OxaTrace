#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>

#include <typeinfo>

using namespace oxatrace;

static double const MIN_IMPORTANCE = 0.0001;
unsigned const      MAX_DEPTH = 16;

color
shade(scene const& scene, ray const& ray,
      color background = {0.0, 0.0, 0.0},
      unsigned depth = 0, double importance = 1.0) {
  if (importance < MIN_IMPORTANCE || depth > MAX_DEPTH)
    return background;

  boost::optional<scene::intersection> i = scene.intersect_solid(ray);
  if (!i)
    return background;

  color result{i->solid().material().base_color()};
  for (light const& l : scene.lights()) {
    unit3 const light_dir{l.get_source() - i->position()};

    if (auto obstacle = scene.intersect_solid({i->position(), light_dir}))
      if ((obstacle->position() - i->position()).squaredNorm() <
          (l.get_source() - i->position()).squaredNorm())
        continue;  // Obstacle blocks direct path from light to solid

    result = i->solid().material().add_light(
      result, i->normal(), l.color(), light_dir
    );
  }

  unit3 reflection_dir = reflect(ray.direction(), i->normal());
  oxatrace::ray reflected{i->position(), reflection_dir};
  double const reflection_importance = i->solid().material().reflectance();
  color const reflection = shade(scene, reflected, background,
                                 depth + 1, reflection_importance * importance);
  result = i->solid().material().add_reflection(result, reflection);

  return result;
}

int
main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Expected a filename, sorry.\n";
    return EXIT_FAILURE;
  }

  char const* filename{argv[1]};

  std::cout << "Building scene...\n";

  scene_definition def;
  auto sphere_shape = std::make_shared<oxatrace::sphere>();

  solid sphere1{sphere_shape,
                material{color{0.2, 0.2, 0.2}, 0.4, 0.8, 50, 0.2}};
  sphere1
    .scale(3.0)
    .translate({0, 3, -5})
    ;
  def.add_solid(std::move(sphere1));

  solid sphere2{sphere_shape,
                material{color{0.2, 0.2, 0.2}, 0.4, 0.8, 50, 0.2}};
  sphere2
    .scale(3.0)
    .translate({-8, 3, -2})
    ;
  def.add_solid(std::move(sphere2));

  solid plane{std::make_shared<oxatrace::plane>(),
              material{color{0.1, 0.1, 0.1}, 0.5, 0.8, 200, 0.2}};
  plane
    .rotate(Eigen::AngleAxisd{PI / 2., vector3::UnitX()})
    ;
  def.add_solid(std::move(plane));

  def.add_light(
    std::make_shared<point_light>(vector3{-6.0, 10.0, 8.0},
                                  color{1.0, 0.8, 0.8})
  );
  
  std::unique_ptr<scene> sc{simple_scene::make(std::move(def))};
  camera cam{vector3{3.0, 3.5, 8.0}, unit3{-0.2, -0.3, -0.7},
             unit3{0.0, 1.0, 0.0}, 640, 480, PI / 2.0};
  
  std::cout << "Tracing rays...\n";
  
  image result{640, 480};
  for (image::index y = 0; y < result.height(); ++y)
    for (image::index x = 0; x < result.width(); ++x) {
      double const cam_u = double(x) / double(result.width());
      double const cam_v = double(y) / double(result.height());
      
      ray const r = cam.make_ray(cam_u, cam_v);
      result.pixel_at(x, y) = shade(*sc, r);
    }

  std::cout << "Saving result image...\n";
  save(result, filename);

  std::cout << "Done\n";
}

