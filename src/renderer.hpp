#ifndef OXATRACE_SHADER_HPP
#define OXATRACE_SHADER_HPP

#include "color.hpp"
#include "math.hpp"

namespace oxatrace {

class scene;

// Specifies how shading is to be carried out. This contains the default
// background colour as well as conditions when to stop the recursive
// raytracing process.
class shading_policy {
public:
  // Observers...

  // Decide whether raytracing should continue.
  bool
  should_continue(unsigned current_depth, double current_importance) const;

  // Get the default background
  hdr_color
  background() const noexcept { return background_; }

  // Setting values...

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
};

// Takes care of tracing rays through the scene and returning the collected
// colour of a ray.
hdr_color
shade(scene const& scene, ray const& ray, shading_policy const& policy);

}

#endif
