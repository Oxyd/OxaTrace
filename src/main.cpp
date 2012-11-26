#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>

#include <typeinfo>

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    std::cerr << "Expected a filename, sorry.\n";
    return EXIT_FAILURE;
  }

  char const* filename{argv[1]};

  using namespace oxatrace;

  std::cout << "Building the scene...\n";

  scene_definition def;
  solid sphere{std::make_shared<oxatrace::sphere>(),
               material{color{0.2, 0.2, 0.2}, 0.4, 0.8, 50}};
  sphere
    .scale(3.0)
    .translate({0, 3, -5})
    ;
  def.add_solid(std::move(sphere));

  solid plane{std::make_shared<oxatrace::plane>(),
              material{color{0.6, 0.6, 0.6}, 0.5, 0.2, 200}};
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

      boost::optional<scene::intersection> i = sc->intersect_solid(r);
      if (!i) {
        result.pixel_at(x, y) = {0.0, 0.0, 0.0};
        continue;
      }

      color result_pixel{i->solid().material().base_color()};
      for (light const& l : sc->lights()) {
        unit<vector3> const light_dir = {l.get_source() - i->position()};
        if (auto obstacle = sc->intersect_solid({i->position(), light_dir}))
          if ((obstacle->position() - i->position()).squaredNorm() <
              (l.get_source() - i->position()).squaredNorm())
            continue;

        result_pixel = i->solid().material().illuminate(
          result_pixel, i->normal(), l, light_dir
        );
      }

      result.pixel_at(x, y) = result_pixel;
    }

  std::cout << "Saving result image...\n";
  save(result, filename);
}

