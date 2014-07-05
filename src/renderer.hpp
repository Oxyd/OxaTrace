#ifndef OXATRACE_SHADER_HPP
#define OXATRACE_SHADER_HPP

#include "color.hpp"
#include "math.hpp"

namespace oxatrace {

// Specifies how shading is to be carried out.
//
// This contains the background colour as well as a condition when to stop the
// recursive ray tracing process.
//
// Stop condition is based on maximum recursion depth and minimal ray
// importance: Recursion will stop if it has either gone too deep or when
// sampling an additional ray would contribute too little to the overall result.
class shading_policy {
public:
  bool
  should_continue(unsigned current_depth, double current_importance) const;

  hdr_color
  background() const noexcept { return background_; }

  void
  background(hdr_color const& new_background) noexcept {
    background_ = new_background;
  }

  void
  max_depth(unsigned new_max_depth) noexcept {
    max_depth_ = new_max_depth;
  }

  // Set minimal importance for importance sampling.
  // Throws: std::invalid_argument new_min_importance isn't in [0, 1].
  void
  min_importance(double new_min_importance);

private:
  hdr_color background_     = {0.0, 0.0, 0.0};
  unsigned  max_depth_      = 16;
  double    min_importance_ = EPSILON;
};

class scene;

// Trace rays.
//
// Recursively traces rays through the given scene originating with the given
// ray. The process stops then the policy tells it to stop.
hdr_color
shade(scene const& scene, ray const& ray, shading_policy const& policy);

class camera;

// Sample a pixel of the image.
hdr_color
sample(scene const& scene, camera const& cam, rectangle pixel,
       shading_policy const& policy);

}

#endif
