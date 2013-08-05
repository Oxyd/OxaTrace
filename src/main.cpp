#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <typeinfo>

using namespace oxatrace;

static double const MIN_IMPORTANCE = 0.0001;
unsigned const      MAX_DEPTH = 16;

hdr_color
shade(scene const& scene, ray const& ray,
      hdr_color background = {0.0, 0.0, 0.0},
      unsigned depth = 0, double importance = 1.0) {
  if (importance < MIN_IMPORTANCE || depth > MAX_DEPTH)
    return background;

  boost::optional<scene::intersection> i = scene.intersect_solid(ray);
  if (!i)
    return background;

  hdr_color result{i->solid().material().base_color()};
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
  hdr_color const reflection = shade(scene, reflected, background,
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
  hdr_color const sphere_color{0.8, 0.55, 0.75};
  material const sphere_material{sphere_color, 0.1, 0.6, 100, 0.2};

  solid sphere1{sphere_shape, sphere_material};
  sphere1
    .scale(3.0)
    .translate({0, 3, -15})
    ;
  def.add_solid(std::move(sphere1));

  solid sphere2{sphere_shape, sphere_material};
  sphere2
    .scale(3.0)
    .translate({-8, 3, -15})
    ;
  def.add_solid(std::move(sphere2));

  solid plane{std::make_shared<oxatrace::plane>(),
              material{hdr_color{0.7, 0.7, 0.7}, 0.2, 0.5, 300, 0.1}};
  plane
    .rotate(Eigen::AngleAxisd{PI / 2., vector3::UnitX()})
    ;
  def.add_solid(std::move(plane));

  def.add_light(
    std::make_shared<point_light>(vector3{-6.0, 10.0, 8.0},
                                  hdr_color{0.9, 0.9, 0.9})
  );

  std::unique_ptr<scene> sc{simple_scene::make(std::move(def))};

  camera cam{640.0 / 480.0, PI / 2.0};
  cam
    .rotate(Eigen::AngleAxisd{-PI / 18, vector3::UnitX()})
    .rotate(Eigen::AngleAxisd{PI / 15, vector3::UnitY()})
    .translate({0.0, 4.0, 0.0})
    ;
  
  std::cout << "Tracing rays...\n";
  
  hdr_image result{640, 480};

  unsigned total = 640 * 480;
  unsigned done = 0;
  unsigned last_perc = 0;

  for (hdr_image::index y = 0; y < result.height(); ++y)
    for (hdr_image::index x = 0; x < result.width(); ++x) {
      double const cam_u = double(x) / double(result.width());
      double const cam_v = double(y) / double(result.height());
      
      ray const r = cam.make_ray(cam_u, cam_v);
      result.pixel_at(x, y) = shade(*sc, r);

      ++done;
      double complete = double(done) / double(total) * 100;
      unsigned const PROGRESS_WIDTH = 40;

      if (unsigned(complete * 100) != last_perc) {
        std::cout << '\r'
                  << std::setfill(' ') << std::setw(6)
                  << std::fixed << std::setprecision(2)
                  << complete << "% [";
        unsigned progress = double(done) / double(total) * PROGRESS_WIDTH;
        for (unsigned i = 0; i < progress; ++i) std::cout << '#';
        for (unsigned i = progress; i < PROGRESS_WIDTH; ++i) std::cout << ' ';
        std::cout << ']' << std::flush;
        last_perc = unsigned(complete * 100);
      }
    }

  std::cout << "\nSaving result image...\n";
  auto ldr_result = transform(result, clip);
  save(ldr_result, filename);

  std::cout << "Done\n";
}

