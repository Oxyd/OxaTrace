#include "renderer.hpp"

#include "camera.hpp"
#include "math.hpp"
#include "scene.hpp"
#include "solids.hpp"

#include <array>
#include <numeric>

using namespace oxatrace;

static hdr_color
blend_light(
  material const& material,
  hdr_color const& base_color, unit3 const& normal,
  hdr_color const& light_color, vector3 const& light_dir
) {
  // We're using the Phong shading model here, which is an empiric one without
  // much basis in real physics. Aside from the ambient term (which is there
  // to simulate background light which "just happens" in real life), we have
  // the diffuse and specular terms. Each of these two is weighted by the
  // two respective parameters of the constructor. The intensity of diffuse
  // or specular highlight depends on how directly the light shines on the
  // given surface -- in other words, the cosine of the angle between surface
  // normal and the direction of the light source.
  //
  // Together, we have the formula for the intensity of one light source:
  //
  //                                                   specular_exponent
  //   I = diffuse * cos(alpha) + specular * cos(alpha),
  //
  // To add colours into the mix, we then multiply the light's colour with
  // its computed intensity.
  //
  // XXX: This should take distance to the light source into account as well.

  double const cos_alpha = cos_angle(normal, light_dir);

  if (cos_alpha <= 0.0) return material.base_color();

  hdr_color const diffuse_color = light_color * material.diffuse() * cos_alpha;
  hdr_color const specular_color =
    light_color * material.specular()
    * std::pow(cos_alpha, material.specular_exponent());
    
  return base_color + diffuse_color + specular_color;
}

static hdr_color
blend_reflection(material const& material, hdr_color const& base_color,
                 hdr_color const& reflection_color)
{
  return base_color + reflection_color * material.reflectance();
}

static bool
should_continue(unsigned current_depth, double current_importance,
                shading_policy const& policy) {
  if (current_importance < 0.0 || current_importance > 1.0)
    throw std::logic_error{"should_continue: importance outside [0, 1]"};

  return current_depth <= policy.max_depth
    && current_importance >= policy.min_importance;
}

static hdr_color
do_shade(scene const& scene, ray const& ray, shading_policy const& policy,
         unsigned depth, double importance, sampler_prng_engine& prng)
{
  if (!should_continue(depth, importance, policy))
    return policy.background;

  boost::optional<scene::intersection> i = scene.intersect_solid(ray);
  if (!i)
    return policy.background;

  hdr_color result = i->texture();
  for (light const& l : scene.lights()) {
    vector3 const light_dir{l.get_source() - i->position()};

    if (auto obstacle = scene.intersect_solid({i->position(), light_dir}))
      if ((obstacle->position() - i->position()).squaredNorm() <
          (l.get_source() - i->position()).squaredNorm())
        continue;  // Obstacle blocks direct path from light to solid

    result = blend_light(
      i->solid().material(), result, i->normal(), l.color(), light_dir
    );
  }

  unit3 const perfect_reflection_dir = reflect(ray.direction(), i->normal());
  unit3 const reflection_dir = cos_lobe_perturb(
    perfect_reflection_dir,
    i->solid().material().specular_exponent(),
    prng
  );
  oxatrace::ray const reflected{i->position(), reflection_dir};
  double const reflection_importance = i->solid().material().reflectance();
  hdr_color const reflection = do_shade(
    scene, reflected, policy, depth + 1, reflection_importance * importance,
    prng
  );
  result = blend_reflection(i->solid().material(), result, reflection);

  return result;
}

static hdr_color
shade(scene const& scene, ray const& ray,
      shading_policy const& policy, sampler_prng_engine& prng) {
  return do_shade(scene, ray, policy, 0, 1.0, prng);
}

// A subpixel is subdivided into four further subpixels, like so:
//
//   +-----+
//   |  |  |
//   +--+--+
//   |  |  |
//   +-----+
//
// When sampling a subpixel, we'll send a ray through each of the four corners.
// If the resulting colours differ too much, we'll then repeat the process
// recursively on each of the four subpixels. This process stops at a depth
// given by shading_policy::supersampling.
//
// In order to only trace as many pixels as necessary, we'll first divide
// the entire pixel into the apropriate number of subpixels (supersampling^2 of
// them), and each of these subpixels gets a slot in an array. Every time a ray
// is traced, the result is stored in this array. When subpixel_sample is called
// recursively, one of the four corners of the current subpixel may already
// have been sampled -- the result will be found in the array and there won't be
// any need to sample it again.

namespace {
  class subpixel_ref;

  // This is a container for pixel subsamples. The bulk of the job is handled
  // by subpixel.
  class pixel_samples {
  public:
    struct sample {
      hdr_color value;
      unsigned  weight;
    };
    
    using sample_list = std::vector<sample>;

    pixel_samples();
    pixel_samples(rectangle pixel, unsigned side);

    void reset(rectangle pixel, unsigned side);
    
    sample_list::reference  at(unsigned x, unsigned y);
    sample_list::value_type at(unsigned x, unsigned y) const;
    
    sample_list::reference add(vector2 point, sample sample);

    sample_list::const_iterator begin() const { return samples_.begin(); }
    sample_list::const_iterator end() const   { return samples_.end(); }
    sample_list::size_type      size() const  { return samples_.size(); }

    rectangle region() const { return region_; }
    unsigned  side() const   { return side_; }

  private:
    sample_list samples_;
    rectangle   region_;
    unsigned    side_;
  };

  class subpixel_ref {
  public:
    static unsigned constexpr top_left     = 0;
    static unsigned constexpr top_right    = 1;
    static unsigned constexpr bottom_left  = 2;
    static unsigned constexpr bottom_right = 3;
    static std::array<unsigned, 4> constexpr corners{{
      top_left, top_right, bottom_left, bottom_right
    }};

    subpixel_ref(pixel_samples& samples);
    subpixel_ref(pixel_samples& samples,
                 unsigned x, unsigned y, unsigned side);

    boost::optional<pixel_samples::sample&>
    get_any();

    unsigned total_weight() const;

    rectangle region() const;
    unsigned  side() const { return side_; }

    subpixel_ref corner(unsigned corner) const;

  private:
    pixel_samples& samples_;
    unsigned       offset_x_, offset_y_;
    unsigned       side_;
  };
}

pixel_samples::pixel_samples()
  : side_{0}
{ }

pixel_samples::pixel_samples(rectangle pixel, unsigned supersampling)
  : pixel_samples()
{
  reset(pixel, supersampling);
}

void
pixel_samples::reset(rectangle pixel, unsigned side) {
  assert(is_power2(side));
  
  samples_.clear();
  samples_.resize(side * side);
  region_ = pixel;
  side_ = side;
}

auto
pixel_samples::at(unsigned x, unsigned y) -> sample_list::reference {
  return samples_[y * side_ + x];
}

auto
pixel_samples::at(unsigned x, unsigned y) const -> sample_list::value_type {
  return samples_[y * side_ + x];
}

auto
pixel_samples::add(vector2 point, sample sample) -> sample_list::reference {
  vector2 const offset = point - region_.top_left();
  assert(offset.x() >= 0.0 && offset.x() < region_.width());
  assert(offset.y() >= 0.0 && offset.y() < region_.height());

  unsigned const x = (unsigned) (offset.x() * side_ / region_.width());
  unsigned const y = (unsigned) (offset.y() * side_ / region_.height());

  assert(x < side_);
  assert(y < side_);
  
  assert(samples_[y * side_ + x].weight == 0);
  samples_[y * side_ + x] = sample;

  return samples_[y * side_ + x];
}

decltype(subpixel_ref::corners) constexpr subpixel_ref::corners;

subpixel_ref::subpixel_ref(pixel_samples& samples)
  : subpixel_ref(samples, 0, 0, samples.side()) { }

subpixel_ref::subpixel_ref(pixel_samples& samples, unsigned x, unsigned y,
                   unsigned side)
  : samples_{samples}
  , offset_x_{x}
  , offset_y_{y}
  , side_{side}
{
  assert(is_power2(side));
  assert(offset_x_ < samples_.side());
  assert(offset_y_ < samples_.side());
  assert(within(region(), samples_.region()));
}

boost::optional<pixel_samples::sample&>
subpixel_ref::get_any() {
  for (unsigned x = offset_x_; x < offset_x_ + side_; ++x)
    for (unsigned y = offset_y_; y < offset_y_ + side_; ++y) {
      auto& sample = samples_.at(x, y);
      if (sample.weight > 0) return sample;
    }

  return {};
}

unsigned
subpixel_ref::total_weight() const {
  unsigned weight{};
  for (unsigned x = offset_x_; x < offset_x_ + side_; ++x)
    for (unsigned y = offset_y_; y < offset_y_ + side_; ++y)
      weight += samples_.at(x, y).weight;
  return weight;
}

rectangle
subpixel_ref::region() const {
  double const w = samples_.region().width() / samples_.side();
  double const h = samples_.region().height() / samples_.side();
  double const width = side_ * w;
  double const height = side_ * h;
  double const x = samples_.region().x() + offset_x_ * w;
  double const y = samples_.region().y() + offset_y_ * h;
  return {x, y, width, height};
}

subpixel_ref
subpixel_ref::corner(unsigned c) const {
  assert(c >= corners.front() && c <= corners.back());
  assert(side_ > 1);

  unsigned s = side_ / 2;

  return {samples_, offset_x_ + s * (c % 2), offset_y_ + s * (c / 2), s};
}

namespace {
  // Result of sampling a point in a pixel.
  struct sample_result {
    hdr_color color;
    vector2   point;  // The point the sample was taken from, relative to the
                      // pixel's top left corner.
  };
}

// Take exactly one sample from the given pixel. Selects a point uniformly
// randomly from within the pixel and traces a ray through it.
static pixel_samples::sample&
sample_one(scene const& scene, camera const& cam, rectangle pixel,
           shading_policy const& policy, unsigned weight,
           pixel_samples& samples,
           sampler_prng_engine& prng)
{
  double const x_mu = pixel.width() / 2;
  double const y_mu = pixel.height() / 2;

  double const x_w = pixel.width() / 4;
  double const y_w = pixel.height() / 4;
  
  std::uniform_real_distribution<> x_jitter_distrib{-x_w, +x_w};
  std::uniform_real_distribution<> y_jitter_distrib{-y_w, +y_w};

  vector2 const offset =
    policy.jitter
      ? vector2{x_mu + x_jitter_distrib(prng), y_mu + y_jitter_distrib(prng)}
      : vector2{x_mu, y_mu}
      ;
  vector2 const point = pixel.top_left() + offset;
  hdr_color const color = shade(scene, cam.make_ray(point), policy, prng);

  return samples.add(point, {color, weight});
}

// Sample a rectangular sub-pixel, recursing as necessary.
static void
subpixel_sample(scene const& scene, camera const& cam,
                shading_policy const& policy, subpixel_ref pixel,
                pixel_samples& samples, sampler_prng_engine& prng)
{
  unsigned const weight = pixel.side() * pixel.side();
  unsigned const weight_4 = weight / 4;

  if (pixel.side() == 1) {
    // No further subdivision of this subpixel.
    boost::optional<pixel_samples::sample&> sample = pixel.get_any();
    if (!sample)
      sample_one(scene, cam, pixel.region(), policy, weight, samples, prng);
    else
      sample->weight = weight;

    assert(pixel.total_weight() == weight);
    return;
  }

  auto const max_channel = std::numeric_limits<hdr_color::channel>::max();
  auto const min_channel = std::numeric_limits<hdr_color::channel>::min();
  hdr_color min{max_channel, max_channel, max_channel};
  hdr_color max{min_channel, min_channel, min_channel};

  for (auto corner_index : subpixel_ref::corners) {
    subpixel_ref corner = pixel.corner(corner_index);
    boost::optional<pixel_samples::sample&> sample = corner.get_any();

    if (!sample)
      sample = sample_one(scene, cam, corner.region(), policy,
                          weight_4, samples, prng);
    else
      sample->weight = weight_4;
    
    assert(corner.get_any());
    assert(sample);
    
    for (std::size_t channel = 0; channel < hdr_color::CHANNELS; ++channel) {
      if (sample->value[channel] > max[channel])
        max[channel] = sample->value[channel];
      if (sample->value[channel] < min[channel])
        min[channel] = sample->value[channel];
    }
  }

  assert(pixel.total_weight() == weight);

  double const max_distance = 0.2;
  double const dist = distance(min, max);

  if (dist > max_distance) {
    for (auto corner_index : subpixel_ref::corners)
      subpixel_sample(scene, cam, policy, pixel.corner(corner_index),
                      samples, prng);
  } 
}

hdr_color
oxatrace::sample(scene const& scene, camera const& cam, rectangle pixel,
                 shading_policy const& policy, sampler_prng_engine& prng) {
  static thread_local pixel_samples samples;
  samples.reset(pixel, policy.supersampling);
  subpixel_sample(scene, cam, policy, {samples}, samples, prng);

  assert(std::accumulate(samples.begin(), samples.end(), 0u,
                         [] (unsigned accum, pixel_samples::sample s) {
                           return accum + s.weight;
                         }) == samples.size());

  hdr_color sum{{}, {}, {}};
  for (auto const& sample : samples)
    sum += sample.value * sample.weight;

  return sum / samples.size();
}
