#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>

auto main(int argc, char** argv) -> int {
  if (argc != 2) {
    std::cerr << "Expected a filename, sorry.\n";
    return EXIT_FAILURE;
  }

  char const* filename{argv[1]};

  using namespace oxatrace;

  scene_definition def;
  def.add_solid(
    solid{std::make_shared<sphere>(vector3{1.0, 1.0, -5.0}, 3.0),
          material{color{0.2, 0.2, 0.2}, 0.4, 0.8, 50}}
  );
  def.add_solid(
    solid{std::make_shared<plane>(vector3{0.0, -2.0, 0.0},
                                  vector3{0.0, 1.0, 0.0}),
          material{color{0.6, 0.6, 0.6}, 0.5, 0.2, 200}}
  );
  def.add_light(
    std::make_shared<point_light>(vector3{-1.0, 3.0, 0.0},
                                  color{1.0, 0.8, 0.8})
  );

  std::unique_ptr<scene> sc{simple_scene::make(std::move(def))};
  camera cam{{1.0, 3.5, 5.0}, {0.0, -0.3, -0.7}, {0.0, 1.0, 0.0},
             640, 480, PI / 2.0};

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
        if (sc->intersect_solid({i->position(), light_dir}))
          continue;

        if (dot(i->normal().get(), light_dir.get()) < 0.0) {
          std::cerr << "i->normal = " << i->normal().get() << '\n'
                    << "light_dir = " << light_dir.get() << '\n'
                    << "l.get_source() = " << l.get_source() << '\n'
                    << "i->position() = " << i->position() << '\n'
                    << "l.get_source() - i->position = " << (l.get_source() - i->position()) << '\n';
          assert(!"Now, die.");
        }

        result_pixel = i->solid().material().illuminate(
          result_pixel, i->normal(), l, light_dir
        );
      }

      result.pixel_at(x, y) = result_pixel;
    }

  save(result, filename);
}

