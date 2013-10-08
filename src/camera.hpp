#ifndef OXATRACE_CAMERA_HPP
#define OXATRACE_CAMERA_HPP

#include "math.hpp"

#include <Eigen/Geometry>

namespace oxatrace {

/// \brief A source of rays.
/// \ingroup scene
///
/// This models a simple pinhole camera. In its default position, the camera
/// points along the vector \f$(0, 0, 1)\f$ and sits in the origin. It may be
/// moved and rotated using the translate and rotate member functions.
class camera {
public:
  /// \throws std::out_of_range field_of_view is not in \f$(0, \pi)\f$.
  camera(double aspect_ratio, double field_of_view);

  /// \name Ray generation
  ///@{

  /// \brief Creates a ray corresponding to a position \f$(u, v)\f$ on the film.
  /// \throws std::out_of_range \f$(u, v)\f$ not in \f$[0, 1]^2\f$.
  ray
  make_ray(double u, double v) const;
  ///@}

  /// \name Camera coordinate transformation...
  ///@{

  /// \brief Translate the camera in space.
  camera&
  translate(vector3 const& tr);

  /// \brief Rotate the camera in space.
  camera&
  rotate(Eigen::AngleAxisd const& rot);
  ///@}

private:
  Eigen::Affine3d   camera_to_world_;
  double            film_max_x_;
  double            film_max_y_;
};

} // namespace oxatrace

#endif

