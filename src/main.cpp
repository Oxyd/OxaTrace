#include "camera.hpp"
#include "image.hpp"
#include "lights.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "text_interface.hpp"
#include "util.hpp"

#include <boost/program_options.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

using namespace oxatrace;

scene_definition
two_balls() {
  scene_definition def;
  auto sphere_shape = std::make_shared<oxatrace::sphere>();
  auto plane_shape = std::make_shared<oxatrace::plane>();

  auto plane_checker = std::make_shared<oxatrace::checkerboard>(
    hdr_color{0.7, 0.7, 0.7}, hdr_color{0.8, 0.1, 0.1}
  );
  
  hdr_color const sphere_color{0.4, 0.4, 0.6};
  material const sphere_material{sphere_color, 0.4, 0.9, 50, 0.4};

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

  material const plane_material{hdr_color{0.5, 0.5, 0.5}, 0.5, 0.5, 200, 0.2};
  auto plane = make_unique<solid>(plane_shape, plane_material, plane_checker);
  (*plane)
    .scale(3.0)
    .rotate(Eigen::AngleAxisd{PI / 2., vector3::UnitX()})
    ;
  
  def.add_solid(std::move(plane));

  def.add_light(
    make_unique<point_light>(vector3{-6.0, 10.0, 8.0},
                             hdr_color{1.0, 1.0, 1.0})
  );

  return def;
}

scene_definition
textured_ball() {
  scene_definition def;
  auto sphere_shape = std::make_shared<sphere>();
  auto checker = std::make_shared<checkerboard>(
    hdr_color{0.9, 0.9, 0.9}, hdr_color{0.1, 0.1, 0.9}, 8
  );
  material const sphere_mat{{0.0, 0.0, 0.0}, 0.6, 0.2, 20, 0.05};

  auto sphere = make_unique<solid>(sphere_shape, sphere_mat, checker);
  (*sphere)
    .scale(3.0)
    .translate({0, 3, -15})
    ;

  def.add_solid(std::move(sphere));

  def.add_light(
    make_unique<point_light>(
      vector3{-6.0, 10.0, 8.0},
      hdr_color{1.0, 1.0, 1.0}
    )
  );

  return def;
}

class renderer_pool {
public:
  renderer_pool(unsigned threads, hdr_image& destination, scene const& scene,
                camera const& camera, shading_policy const& sp)
    : num_threads_(threads)
    , current_job_index_{0}
    , destination_(destination)
    , scene_(scene)
    , camera_(camera)
    , shading_policy_(sp)
  {
    if (num_threads_ == 0)
      throw std::out_of_range{"renderer_pool: Can't do 0 threads"};
  }

  renderer_pool(renderer_pool const&) = delete;

  ~renderer_pool() { finish(); }

  void
  run() {
    for (unsigned i = 0; i < num_threads_; ++i)
      threads_.push_back(std::thread([this] { worker(); }));
  }

  void
  finish() {
    for (auto& thread : threads_)
      thread.join();
    threads_.clear();
  }

  double
  percent_complete() const {
    unsigned const total = total_pixels();
    unsigned const done  = current_job_index_ * job_size;

    if (done <= total)  // It may go over because threads.
      return double(done) / double(total);
    else
      return 1.0;
  }

  bool
  done() const {
    return current_job_index_ * job_size >= total_pixels();
  }

  unsigned
  concurrency() const { return num_threads_; }

private:
  static unsigned constexpr job_size = 1024;

  using job = hdr_image::index;

  unsigned                    num_threads_;
  std::vector<std::thread>    threads_;
  std::atomic<unsigned>       current_job_index_;
  hdr_image&                  destination_;
  scene const&                scene_;
  camera const&               camera_;
  shading_policy              shading_policy_;

  hdr_image::index
  total_pixels() const {
    return destination_.width() * destination_.height();
  }

  bool
  get_job(job& j) {
    if (current_job_index_ * job_size < total_pixels()) {
      std::size_t const index = std::atomic_fetch_add(&current_job_index_, 1u);
      if (index * job_size < total_pixels()) {
        j = index * job_size;
        return true;
      }
    }

    return false;
  }

  void
  worker() {
    std::hash<std::thread::id> hasher;
    sampler_prng_engine prng{hasher(std::this_thread::get_id())};
    double const pixel_width  = 1.0 / destination_.width();
    double const pixel_height = 1.0 / destination_.height();

    unsigned const total_size = total_pixels();

    while (true) {
      job begin;
      bool const have_job = get_job(begin);
      if (!have_job)
        break;  // And thus end the thread.

      for (hdr_image::index index = begin;
           index < begin + job_size && index < total_size;
           ++index)
      {
        hdr_image::index const x = index % destination_.width();
        hdr_image::index const y = index / destination_.width();

        double const top_left_x = double(x) / double(destination_.width());
        double const top_left_y = double(y) / double(destination_.height());

        assert(0.0 <= top_left_x && top_left_x <= 1.0);
        assert(0.0 <= top_left_y && top_left_y <= 1.0);

        destination_.pixel_at(x, y) = sample(
          scene_, camera_, {top_left_x, top_left_y, pixel_width, pixel_height},
          shading_policy_, prng
        );
      }
    }
  }
};

int
main(int argc, char** argv) try {
  namespace opts = boost::program_options;

  std::size_t width, height;
  std::string filename;
  double gamma;
  unsigned supersampling;
  unsigned threads;

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
    ("threads",
     opts::value<unsigned>(&threads)
       ->default_value(std::thread::hardware_concurrency()),
     "Number of threads to use for rendering")
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

  auto scene_def = &two_balls;
  std::unique_ptr<scene> sc{simple_scene::make(scene_def())};

  camera cam{double(width) / double(height), PI / 2.0};
  cam
    .rotate(Eigen::AngleAxisd{-PI / 18, vector3::UnitX()})
    .rotate(Eigen::AngleAxisd{PI / 15, vector3::UnitY()})
    .translate({0.0, 4.0, 0.0})
    ;
  
  hdr_image result{width, height};
  hdr_color const background{0.05, 0.05, 0.2};

  shading_policy shading_pol;
  shading_pol.background = background;
  shading_pol.jitter = !values["no-jitter"].as<bool>();
  shading_pol.supersampling = supersampling;
  shading_pol.min_importance = 0.01;

  std::chrono::milliseconds const poll_interval{100};

  {
    // Scope is necessary to make sure all threads are joined before moving
    // on.
    
    renderer_pool pool{threads, result, *sc, cam, shading_pol};
    monitor.change_phase(
      std::string{"Tracing rays in "}
      + std::to_string(pool.concurrency()) + " threads..."
    );
    pool.run();
    while (!pool.done()) {
      monitor.update_progress(pool.percent_complete());
      std::this_thread::sleep_for(poll_interval);
    }

    monitor.update_progress(pool.percent_complete());
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

