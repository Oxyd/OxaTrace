#ifndef OXATRACE_CAMERA_HPP
#define OXATRACE_CAMERA_HPP

#include "math.hpp"

namespace oxatrace {

// Camera is a source of rays. It models a simple pinhole camera. It's defined
// by the coordinates of the pinhole, the view and up vectors, field of
// view angle, and aspect ratio of the view.
//
// Rays can be generated by asking for a ray originating from the logical film
// coordinates; these coordinates are from [0, 1]². The (0, 0) coordinate
// corresponds to the bottom-right part of the image, (1, 1) is the 
// upper-left part thereof.
//
// This convention is chosen because the image produced by a pinhole camera
// is inverted about both axes. Using this convention, the ray traced through
// film coordinates of (0, 0) will correspond to the top-left pixel of the
// desired resulting image.
class camera {
public:
  // Construction...
  // Throws:
  //   -- std::out_of_range: field_of_view not in the interval (0, pi).
  camera(vector3 center, unit<vector3> view, unit<vector3> up,
         std::size_t view_width, std::size_t view_height, 
         double field_of_view);

  // Ray generation...
  // Throws:
  //   -- std::out_of_range: (u, v) not in [0, 1]².
  auto make_ray(double u, double v) const -> ray;

private:
  vector3 pin_center_;      // World position of the infinitesimal pinhole.
  vector3 film_bottom_right_;// World position of the (0, 0) corner of the film.
  vector3 film_u_axis_;     // Film's "left" axis.
  vector3 film_v_axis_;     // Film's "up" axis.
};

} // namespace oxatrace

#endif
