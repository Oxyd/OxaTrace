#ifndef OXATRACE_SHADER_HPP
#define OXATRACE_SHADER_HPP

#include "color.hpp"
#include "math.hpp"

namespace oxatrace {

class scene;

/// \defgroup render Rendering

/// \brief Specifies how shading is to be carried out.
/// \ingroup render
///
/// This contains the background colour as well as a condition when to stop
/// the recursive ray tracing process.
///
/// Stop condition is based on maximum recursion depth and minimal ray
/// importance: Recursion will stop if it has either gone too deep or when
/// sampling an additional ray would contribute too little to the overall
/// result.
class shading_policy {
public:
  /// \name Observers
  ///@{

  /// \brief Should ray tracing continue?
  bool
  should_continue(unsigned current_depth, double current_importance) const;

  /// \brief The default background colour.
  hdr_color
  background() const noexcept { return background_; }

  ///@}

  /// \name Modifiers
  ///@{

  /// \brief Set default background.
  void
  background(hdr_color const& new_background) noexcept {
    background_ = new_background;
  }

  /// \brief Set maximum recursion depth.
  void
  max_depth(unsigned new_max_depth) noexcept {
    max_depth_ = new_max_depth;
  }

  /// \brief Set minimal importance for importance sampling.
  /// \throw std::invalid_argument new_min_importance isn't in \f$[0, 1]\f$.
  void
  min_importance(double new_min_importance);

  ///@}

private:
  hdr_color background_     = {0.0, 0.0, 0.0};
  unsigned  max_depth_      = 16;
  double    min_importance_ = EPSILON;
};

/// \brief Trace rays.
/// \ingroup render
///
/// Recursively traces rays through the given scene originating with the given
/// ray. The process stops then the policy tells it to stop.
hdr_color
shade(scene const& scene, ray const& ray, shading_policy const& policy);

}

#endif
