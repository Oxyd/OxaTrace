#ifndef OXATRACE_SHADER_HPP
#define OXATRACE_SHADER_HPP

#include "color.hpp"
#include "math.hpp"

#include <random>

namespace oxatrace {

// Specifies how shading is to be carried out.
//
// This contains the background colour as well as a condition when to stop the
// recursive ray tracing process.
//
// Stop condition is based on maximum recursion depth and minimal ray
// importance: Recursion will stop if it has either gone too deep or when
// sampling an additional ray would contribute too little to the overall result.
struct shading_policy {
  hdr_color background     = {0.0, 0.0, 0.0};
  unsigned  max_depth      = 16;
  double    min_importance = EPSILON;
  bool      jitter         = true;
  unsigned  supersampling  = 2;
};

class scene;
class camera;
using sampler_prng_engine = random_eng;

// Sample a pixel of the image.
hdr_color
sample(scene const& scene, camera const& cam, rectangle pixel,
       shading_policy const& policy, sampler_prng_engine& prng);

}

#endif
