#include "solids.hpp"

#include "color.hpp"
#include "lights.hpp"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <utility>

using namespace oxatrace;

sphere::sphere(vector3 center, double radius)
  : center_{center}
  , radius_{radius} { 
  if (radius <= 0.0) throw std::logic_error("sphere: radius <= 0.0");
}

auto shape::intersect(ray const& r) const -> vector3 {
  return std::get<0>(intersect_both(r));
}

auto sphere::intersect_both(ray const& r) const -> both_intersections {
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
  // a quadratic equation in t. (Here v², where v is a vector, is the scalar
  // product of v with itself.)
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
  // All real and nonnegative t's are then the sought parameters of intersection
  // for the ray formula.
  
  vector3 const a    = r.origin() - center_;
  double const  a_2  = norm_squared(a);
  double const  ad   = dot(a, r.direction().get());
  double const  ad_2 = ad * ad;
  double const  d_2  = norm_squared(r.direction().get());
  double const  r_2  = radius_ * radius_;

  double const D = ad_2 - d_2 * (a_2 - r_2);

  both_intersections result{vector3::zero(), vector3::zero()};
  if (D >= 0.0) {
    double const sqrt_D = std::sqrt(D);
    double t_1          = -ad + sqrt_D;
    double t_2          = -ad - sqrt_D;

    // Only consider nonnegative values for t₁, t₂, and make t₁ ≤ t₂.
    if (t_1 > t_2) std::swap(t_1, t_2);
    if (t_1 <= EPSILON) t_1 = t_2;

    if (t_1 > EPSILON) {  // t_1 == 0 means we started out on the shape.
      std::get<0>(result) = point_at(r, t_1);
      if (double_neq(t_1, t_2))
        std::get<1>(result) = point_at(r, t_2);
    }
  }

  return result;
}

auto sphere::normal_at(vector3 point) const -> unit<vector3> {
  return point - center_;
}

material::material(color const& ambient, double diffuse, double specular,
                   unsigned specular_exponent)
  : ambient_{ambient}
  , diffuse_{diffuse}
  , specular_{specular} 
  , specular_exponent_{specular_exponent} { 
  if (diffuse < 0.0 || diffuse > 1.0)
    throw std::invalid_argument{"material: Invalid diffuse coefficient."};
  if (specular < 0.0 || specular > 1.0)
    throw std::invalid_argument{"material: Invalid specular coefficient."};
}

auto material::base_color() const -> color { return ambient_; }

auto material::illuminate(
  color const& base_color, unit<vector3> const& normal,
  light const& light, unit<vector3> const& light_dir
) const -> color {
  // We're using the Phong shading model here, which is an empiric one without
  // much basis in real physics. Aside from the ambient term (which is there
  // to simulate background light which "just happens" in real life), we have
  // the diffuse and specular terms. Each of these two is weighted by the
  // two respective parameters of the constructor. The intensity of diffuse
  // or specular highlight depends on how directly the light shines on the
  // given surface -- in other words, the cosine of the angle between surface
  // normal and the direction of the light source.
  //
  // Together, we have the formula for the intensity of one light source:
  //
  //                                 specular_exponent
  //   I = diffuse ∙ x + specular ∙ x,
  //
  // Where x := max { N ∙ L, 0 }, N is the surface normal and L is the
  // direction toward the light source. We need to clip x to be nonnegative
  // here to avoid weird results when the angle between N and L is more than
  // 90 degrees.
  //
  // To add colours into the mix, we then multiply the light's colour with
  // its computed intensity.
  //
  // XXX: This should take distance to the light source into account as well.

  double const x = std::max(dot(normal.get(), light_dir.get()), 0.0);
  color const diffuse_color{light.color() * diffuse_ * x};
  color const specular_color{
    light.color() * specular_ * std::pow(x, specular_exponent_)
  };

  return base_color + diffuse_color + specular_color;
}

solid::solid(std::shared_ptr<oxatrace::shape> const& s, oxatrace::material mat)
  : shape_{std::move(s)}
  , material_{mat} { }

auto solid::shape() const noexcept -> oxatrace::shape const& {
  return *shape_;
}

auto solid::material() const noexcept -> oxatrace::material const& {
  return material_;
}

