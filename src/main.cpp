#include "image.hpp"
#include "scene.hpp"
#include "camera.hpp"

#include <algorithm>
#include <iostream>

auto main(int argc, char** argv) -> int {
  if (argc == 2) {
    char const* filename{argv[1]};

    using namespace oxatrace;

    scene_definition def;
    def.add_solid(
      solid{std::make_shared<sphere>(vector3{1.0, 1.0, -5.0}, 3.0)}
    );

    std::unique_ptr<scene> sc{simple_scene::make(std::move(def))};
    camera cam{{1.0, 1.0, 2.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0},
               640, 480, PI / 2.0};

    image result{640, 480};
    for (image::index y = 0; y < result.height(); ++y)
      for (image::index x = 0; x < result.width(); ++x) {
        double const cam_u = double(x) / double(result.width());
        double const cam_v = double(y) / double(result.height());

        ray const r = cam.make_ray(cam_u, cam_v);
        if (sc->intersect_solid(r))
          result.pixel_at(x, y) = {1.0, 1.0, 1.0};
        else
          result.pixel_at(x, y) = {0.0, 0.0, 0.0};
      }

    save(result, filename);

  } else std::cerr << "Expected a filename, sorry.\n";
}

