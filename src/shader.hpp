#ifndef OXATRACE_SHADER_HPP
#define OXATRACE_SHADER_HPP

#include "color.hpp"
#include "math.hpp"

namespace oxatrace {

class scene;

// Takes care of tracing rays through the scene and returning the collected
// colour of a ray.
//
// This consist of configuration variables which tell the default colour in
// case the ray doesn't hit any object (background), maximum recursion depth,
// and minimal ray importance for it to be traced further
class shader {
public:
  // Trace a ray through a scene and return the resulting pixel colour.
  hdr_color
  shade(scene const& scene, ray const& ray) const {
    return do_shade(scene, ray, 0, 1.0);
  }

  // Configuration...

  void
  background(hdr_color const& new_background) noexcept {
    background_ = new_background;
  }

  void
  max_depth(unsigned new_max_depth) noexcept {
    max_depth_ = new_max_depth;
  }

  // Throws std::invalid_argument if new_min_importance isn't in range [0, 1].
  void
  min_importance(double new_min_importance);

private:
  hdr_color background_     = {0.0, 0.0, 0.0};
  unsigned  max_depth_      = 16;
  double    min_importance_ = EPSILON;

  hdr_color
  do_shade(scene const& scene, ray const& ray,
           unsigned depth, double importance) const;
};

}

#endif
