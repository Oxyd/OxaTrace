#include "camera.hpp"

#include <cmath>
#include <stdexcept>

using namespace oxatrace;

camera::camera(vector3 center, unit3 view, unit3 up,
               std::size_t view_width, std::size_t view_height,
               double field_of_view) {
  // 
  //   \     /
  //    \   /
  //     \ /
  //      X     ← pinhole
  //     / \                                                                   |
  //    /   \                                                                  |
  //   /     \                                                                 |
  //  --------- ← film
  //
  // The figure above depicts the situation we want to model. Pinhole's 
  // coordinates are given. The film is a subspace of the world space -- its
  // origin is its upper-left corner, whose world coordinates are to be 
  // computed. Its bottom-right corner has film coordinates of (1, 1). We also
  // need to compute the two basis vectors of the film.
  //
  // The given angle field_of_view gives the angle (0, 1/2)—pinhole—(1, 1/2).
  // From that, we can compute the world coordinates of (1/2, 1/2) in film's
  // basis. Consider the triangle below:
  //
  //           X
  //          /|
  //         / |
  //        /  | h
  //       /   |
  //      /    |
  //     -------
  // (1, 1/2)   (1/2, 1/2)
  //
  // X is again the pinhole, the other two vertices are given in film 
  // coordinates. To compute (1/2, 1/2) in world coordinates, we need to get
  // the value of the height of the triangle h. (1/2, 1/2) in world coordinates
  // will then be pin_center - h ⋅ view.
  //
  // Denote β := the angle (1/2, 1/2)—pinhole—(1, 1/2); β is half of the 
  // field_of_view angle. Using some trigonometry, we can compute
  //   
  //            1/2     1
  //   tan β = ----- = ---
  //             h     2h
  //
  //              1
  //       h = ------ .
  //           2tan β
  //
  // We can then set film_u_axis := left. Setting the v axis to the camera's
  // up vector would lead to distortion, however, as that would amount to the
  // assumption that the resulting image is to be a square, which is going to
  // be false in almost every invokation of the program. We're therefore going
  // to choose a non-unit vector as the v basis of the film subspace: The
  // choice will be made so that the aspect ratio of the rectangle 
  // (0, 0)—(1, 1) in world space is the same as the aspect ratio of the view,
  // as given by view_width and view_height. This gives us film_v_axis as well.
  //
  // Having found the film centre and its basis vectors, we can then easily
  // compute the film_bottom_right_ using that information.
  //

  if (field_of_view <= 0.0 || field_of_view >= 2.0 * PI)
    throw std::out_of_range{"camera::camera: field_of_view out of range"};

  double const h = 1.0 / (2.0 * std::tan(field_of_view / 2.0));
  unit3 const left = -view.cross(up);

  // Correct the up vector in case the input view and up are not perpendicular.
  unit3 const corrected_up = view.cross(left);

  pin_center_ = center;
  vector3 const film_center = pin_center_ - h * view;
  film_u_axis_ = left;

  double const aspect = double(view_height) / double(view_width);
  film_v_axis_ = corrected_up * aspect;

  film_bottom_right_ = 
    film_center - (1.0 / 2.0) * (film_u_axis_ + film_v_axis_);
}

ray
camera::make_ray(double u, double v) const {
  if (u < 0.0 || v < 0.0 || u >= 1.0 || v >= 1.0)
    throw std::out_of_range{"camera::make_ray: (u, v) out of range"};

  vector3 const film_point{film_bottom_right_ 
                             + u * film_u_axis_ + v * film_v_axis_};
  return {pin_center_, pin_center_ - film_point};
}

