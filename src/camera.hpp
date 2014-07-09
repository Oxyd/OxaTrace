#ifndef OXATRACE_CAMERA_HPP
#define OXATRACE_CAMERA_HPP

#include "math.hpp"

#include <Eigen/Geometry>

namespace oxatrace {

// A source of rays.
//
// This models a simple pinhole camera. In its default position, the camera
// points along the vector (0, 0, 1) and sits in the origin. It may be moved and
// rotated using the translate and rotate member functions.
class camera {
public:
  // Throws std::out_of_range when field_of_view is not in (0, pi).
  camera(double aspect_ratio, double field_of_view);

  // Creates a ray corresponding to a position (u, v) on the film.  Throws
  // std::out_of_range when (u, v) is not in [0, 1]^2.
  ray make_ray(double u, double v) const;
  ray make_ray(vector2 pos) const { return make_ray(pos.x(), pos.y()); }

  // Translate the camera in space.
  camera& translate(vector3 const& tr);

  // Rotate the camera in space.
  camera& rotate(Eigen::AngleAxisd const& rot);

private:
  Eigen::Affine3d   camera_to_world_;
  double            film_max_x_;
  double            film_max_y_;
};

} // namespace oxatrace

#endif

