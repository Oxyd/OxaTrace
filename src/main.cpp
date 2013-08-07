#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"
#include "shader.hpp"
#include "util.hpp"

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <typeinfo>

using namespace oxatrace;

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
  hdr_color const sphere_color{1.0, 0.45, 0.65};
  material const sphere_material{sphere_color, 0.4, 0.8, 100, 0.1};

  auto sphere1 = make_unique<solid>(sphere_shape, sphere_material);
  sphere1->
     scale(3.0)
    .translate({0, 3, -15})
    ;
  def.add_solid(std::move(sphere1));

  auto sphere2 = make_unique<solid>(sphere_shape, sphere_material);
  sphere2->
     scale(3.0)
    .translate({-8, 3, -15})
    ;
  def.add_solid(std::move(sphere2));

  auto plane = make_unique<solid>(
    std::make_shared<oxatrace::plane>(),
    material{hdr_color{0.5, 0.5, 0.5}, 0.5, 0.5, 200, 0.1}
  );
  plane->
     rotate(Eigen::AngleAxisd{PI / 2., vector3::UnitX()})
    ;
  def.add_solid(std::move(plane));

  def.add_light(
    make_unique<point_light>(vector3{-6.0, 10.0, 8.0},
                             hdr_color{1.0, 1.0, 1.0})
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
  hdr_color const background{0.05, 0.05, 0.2};

  shader shader;
  shader.background(background);

  unsigned total = 640 * 480;
  unsigned done = 0;
  unsigned last_perc = 0;

  for (hdr_image::index y = 0; y < result.height(); ++y)
    for (hdr_image::index x = 0; x < result.width(); ++x) {
      double const cam_u = double(x) / double(result.width());
      double const cam_v = double(y) / double(result.height());
      
      ray const r = cam.make_ray(cam_u, cam_v);
      result.pixel_at(x, y) = shader.shade(*sc, r);

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

  double const avg_luminance = log_avg_luminance(result);
  std::cout << avg_luminance << '\n';

  save(
    transform(result, [&] (hdr_image::pixel_type pixel) {
      return to_ldr(gamma_correction()(reinhard(avg_luminance, 0.3)(pixel)));
    }),
    filename
  );

  std::cout << "Done\n";
}

