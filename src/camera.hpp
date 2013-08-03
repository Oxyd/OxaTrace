#ifndef OXATRACE_CAMERA_HPP
#define OXATRACE_CAMERA_HPP

#include "math.hpp"

#include <Eigen/Geometry>

namespace oxatrace {

// Camera is a source of rays. It models a simple pinhole camera. It's defined
// by the coordinates of the pinhole, the view and up vectors, field of
// view angle, and aspect ratio of the view.
class camera {
public:
  // Throws:
  //   -- std::out_of_range: field_of_view not in the interval (0, pi).
  camera(double aspect_ratio, double field_of_view);

  // Ray generation...

  // Throws:
  //   -- std::out_of_range: (u, v) not in [0, 1]².
  ray
  make_ray(double u, double v) const;

  // Camera coordinate transformation...

  camera&
  translate(vector3 const& tr);

  camera&
  rotate(Eigen::AngleAxisd const& rot);

private:
  Eigen::Affine3d   camera_to_world_;
  double            film_max_x_;
  double            film_max_y_;
};

} // namespace oxatrace

#endif

