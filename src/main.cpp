#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "text_interface.hpp"
#include "util.hpp"

#include <boost/program_options.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <string>
#include <sstream>

using namespace oxatrace;

int
main(int argc, char** argv) try {
  namespace opts = boost::program_options;

  std::size_t width, height;
  std::string filename;
  double gamma;
  unsigned supersampling;

  opts::options_description general{"General options"};
  general.add_options()
    ("help", "this cruft")
    ("width,w",
      opts::value<std::size_t>(&width)->default_value(640),
      "width of the result image")
    ("height,h",
      opts::value<std::size_t>(&height)->default_value(480),
      "height of the result image")
    ("output,o",
      opts::value<std::string>(&filename),
      "filename of the output")
    ;

  opts::options_description render{"Rendering options"};
  render.add_options()
    ("no-jitter", opts::bool_switch(), "Disable jittering.")
    ("supersampling,s",
     opts::value<unsigned>(&supersampling)->default_value(4),
     "Supersampling level. Value of 1 disables supersampling. Must be a"
     "power of 2.")
    ;
  
  opts::options_description tone_mapping{"Tone mapping options"};
  tone_mapping.add_options()
    ("no-tone-mapping",
      opts::bool_switch(),
      "Disable tone-mapping entirely")
    ("reinhard,r",
      opts::value<double>()->implicit_value(0.18, "0.18")->value_name("key"),
      "Tone-map the image using Reinhard's operator (this is the default).")
    ("exposure,e",
      opts::value<double>(),
      "Use the exposure operator. The argument corresponds to the exposition "
      "time")
    ("gamma,g",
      opts::value<double>(&gamma)->default_value(2.2, "2.2"),
      "Use this value of gamma for gamma-correction. Value of 0 or 1 disables "
      "gamma-correction.")
    ;

  opts::options_description all_options{"Allowed options"};
  all_options.add(general).add(render).add(tone_mapping);

  opts::variables_map values;
  opts::store(opts::parse_command_line(argc, argv, all_options), values);
  opts::notify(values);

  if (values.count("help")) {
    std::cout << all_options << '\n';
    return EXIT_SUCCESS;
  }

  if (filename.empty())
    throw std::runtime_error{"Output filename must be specified"};

  if (values.count("reinhard") && values.count("exposure"))
    throw std::runtime_error{"Cannot specify both --reinhard and --exposure"};

  if (!is_power2(supersampling))
    throw std::runtime_error{"Supersampling value not a power of 2"};

  std::function<hdr_image(hdr_image)> tone_mapper;
  if (!values["no-tone-mapping"].as<bool>()) {
    if (values.count("exposure")) {
      double e = values["exposure"].as<double>();
      tone_mapper = [e] (hdr_image in) {
        return expose(std::move(in), e);
      };
    } else {
      // Default to Reinhard.
      double r =
        values.count("reinhard") ? values["reinhard"].as<double>() : 0.18;
      tone_mapper = [r] (hdr_image in) {
        return apply_reinhard(std::move(in), r);
      };
    }
  }

  progress_monitor monitor;
  monitor.change_phase("Building scene...");

  scene_definition def;
  auto sphere_shape = std::make_shared<oxatrace::sphere>();
  hdr_color const sphere_color{1.0, 0.45, 0.65};
  material const sphere_material{sphere_color, 0.4, 0.8, 100, 0.1};

  auto sphere1 = make_unique<solid>(sphere_shape, sphere_material);
  (*sphere1)
    .scale(3.0)
    .translate({0, 3, -15})
    ;
  def.add_solid(std::move(sphere1));

  auto sphere2 = make_unique<solid>(sphere_shape, sphere_material);
  (*sphere2)
    .scale(3.0)
    .translate({-8, 3, -15})
    ;
  def.add_solid(std::move(sphere2));

  auto plane = make_unique<solid>(
    std::make_shared<oxatrace::plane>(),
    material{hdr_color{0.5, 0.5, 0.5}, 0.5, 0.5, 200, 0.1}
  );
  (*plane)
    .rotate(Eigen::AngleAxisd{PI / 2., vector3::UnitX()})
    ;
  def.add_solid(std::move(plane));

  def.add_light(
    make_unique<point_light>(vector3{-6.0, 10.0, 8.0},
                             hdr_color{1.0, 1.0, 1.0})
  );

  std::unique_ptr<scene> sc{simple_scene::make(std::move(def))};

  camera cam{double(width) / double(height), PI / 2.0};
  cam
    .rotate(Eigen::AngleAxisd{-PI / 18, vector3::UnitX()})
    .rotate(Eigen::AngleAxisd{PI / 15, vector3::UnitY()})
    .translate({0.0, 4.0, 0.0})
    ;
  
  monitor.change_phase("Tracing rays...");
  
  hdr_image result{width, height};
  hdr_color const background{0.05, 0.05, 0.2};

  shading_policy shading_pol;
  shading_pol.background = background;
  shading_pol.jitter = !values["no-jitter"].as<bool>();
  shading_pol.supersampling = supersampling;

  double const pixel_width  = 1.0 / result.width();
  double const pixel_height = 1.0 / result.height();

  unsigned total = width * height;
  unsigned done  = 0;

  for (hdr_image::index y = 0; y < result.height(); ++y)
    for (hdr_image::index x = 0; x < result.width(); ++x) {
      double const top_left_x = double(x) / double(result.width());
      double const top_left_y = double(y) / double(result.height());
      
      result.pixel_at(x, y) = sample(
        *sc, cam, {top_left_x, top_left_y, pixel_width, pixel_height},
        shading_pol
      );

      monitor.update_progress((double) ++done / (double) total);
    }

  monitor.change_phase("Saving result image...");

  if (tone_mapper)
    result = tone_mapper(std::move(result));
  if (gamma > EPSILON)
    result = correct_gamma(std::move(result), gamma);

  ldr_image const out = ldr_from_hdr(std::move(result));
  save(out, filename);

  monitor.change_phase("Done");
} catch (std::exception& e) {
  std::cerr << "Error: " << e.what() << '\n';
  return EXIT_FAILURE;
}

