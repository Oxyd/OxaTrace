#include "camera.hpp"

#include <cmath>
#include <stdexcept>

#include <Eigen/Geometry>

using namespace oxatrace;

/// \class oxatrace::camera
/// \internal
/// In order to generate rays, we calculate the coordinates of the four
/// corners of our film, and then interpolate between these corners every time,
/// shooting a ray originating on the film and going through the origin of the
/// camera space.

camera::camera(double aspect_ratio, double field_of_view)
  : camera_to_world_{Eigen::Affine3d::Identity()}
{
  if (field_of_view <= 0.0 || field_of_view >= PI)
    throw std::out_of_range{"camera::camera: field_of_view out of range"};

  double const y_fov = field_of_view / aspect_ratio;

  film_max_x_ = std::sin(field_of_view / 2);
  film_max_y_ = std::sin(y_fov / 2);
}

ray
camera::make_ray(double u, double v) const {
  if (u < 0.0 || v < 0.0 || u >= 1.0 || v >= 1.0)
    throw std::out_of_range{"camera::make_ray: (u, v) out of range"};

  // We'll first scale u, v into the range [-1, +1] so that extreme values of
  // u, v give extreme values of our film. Then we'll account for the fact that
  // our film is located behind the pinhole, so without any transformation we'd
  // be getting flipped result. However, the horizontal component doesn't
  // need any flipping as bitmaps and maths have different opinions on the
  // meaning of the y axis.

  vector3 origin{
    film_max_x_ * -2 * (u - 0.5),
    film_max_y_ * +2 * (v - 0.5),
    1.0
  };
  return transform({origin, -origin}, camera_to_world_);
}

camera&
camera::translate(vector3 const& tr) {
  camera_to_world_.pretranslate(tr);
  return *this;
}

camera&
camera::rotate(Eigen::AngleAxisd const& rot) {
  camera_to_world_.prerotate(rot);
  return *this;
}
