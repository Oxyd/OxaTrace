#include "solids.hpp"

#include <cmath>
#include <algorithm>
#include <tuple>

namespace oxatrace {

auto shape::intersect(ray r) const noexcept -> vector3 {
  return std::get<0>(intersect_both(r));
}

auto sphere::intersect_both(ray r) const noexcept -> both_intersections {
  // Let c denote the centre and r the radius. This sphere is defined by the
  // equation ‖x - c‖ = r. Let o := r.origin(), d:= r.direction(), the ray is 
  // then described as x = o + td, (∀t > 0).
  //
  // Substituting the ray equation into the sphere definition gives
  //
  //   ‖o + td - c‖ = r.
  //
  // Define for simplicity a := o - c, Since both sides are non-negative and ‖.‖ 
  // denotes the Euclidian norm, we have
  //
  //                     ‖a + td‖ = r
  //                    ‖a + td‖² = r²
  //                    (a + td)² = r²
  //           a² + 2t(ad) + t²d² = r²
  //    t²d² + t(2ad) + (a² - r²) = 0,
  //
  //  a quadratic equation in t. (Here v², where v is a vector, is the scalar
  //  product of v with itself.)
  //
  // Solving the equation for t gives us the solutions
  //
  //        1           ______________________
  //   t = --- (-2ad ± √4(ad)² - 4(d²(a² - r²))
  //        2
  //
  //        1            ___________________
  //     = --- (-2ad ± 2√(ad)² - d²(a² - r²))
  //        2
  //              ___________________
  //     = -ad ± √(ad)² - d²(a² - r²)
  // 
  // All real and positive t's are then the sought parameters of intersection
  // for the ray formula.
  
  vector3 const a    = r.origin() - center_;
  double const  a_2  = a.squaredNorm();
  double const  ad   = a.dot(r.direction().get());
  double const  ad_2 = ad * ad;
  double const  d_2  = r.direction().get().squaredNorm();
  double const  r_2  = radius_ * radius_;

  double const D = ad_2 - d_2 * (a_2 - r_2);

  both_intersections result{vector3::Zero(), vector3::Zero()};
  if (D >= 0.0) {
    double const sqrt_D = std::sqrt(D);
    double t_1          = -ad + sqrt_D;
    double t_2          = -ad - sqrt_D;

    if (t_1 > t_2) std::swap(t_1, t_2);
    if (t_1 < 0.0) t_1 = t_2;

    if (t_1 >= 0.0) {
      std::get<0>(result) = point_at(r, t_1);
      if (t_2 - t_1 > EPSILON)
        std::get<1>(result) = point_at(r, t_2);
    }
  }

  return result;
}

auto sphere::normal_at(vector3 point) const -> unit<vector3> {
  return unit<vector3>(point - center_);
}

}

// vim:colorcolumn=80
