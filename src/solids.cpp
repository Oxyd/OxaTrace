#include "solids.hpp"

#include "color.hpp"
#include "lights.hpp"

#include <cassert>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <iostream>

using namespace oxatrace;

sphere::sphere(vector3 const& center, double radius)
  : center_{center}
  , radius_{radius} { 
  if (radius <= 0.0) throw std::logic_error("sphere: radius <= 0.0");
}

auto sphere::intersect(ray const& r) const -> intersection_list {
  // Let c denote the centre and r the radius. This sphere is defined by the
  // equation ||x - c|| = r. Let o := r.origin(), d:= r.direction(), the ray is 
  // then described as x = o + td, (forall t > 0).
  //
  // Substituting the ray equation into the sphere definition gives
  //
  //   ||o + td - c|| = r.
  //
  // Define for simplicity a := o - c, Since both sides are non-negative and ‖.‖ 
  // denotes the Euclidian norm, we have
  //
  //                        ||a + td|| = r
  //                      ||a + td||^2 = r^2
  //                        (a + td)^2 = r^2
  //            a^2 + 2t(ad) + t^2 d^2 = r^2
  //    t^2 d^2 + t(2ad) + (a^2 - r^2) = 0,
  //
  // a quadratic equation in t. (Here v^2, where v is a vector, is the scalar
  // product of v with itself.)
  //
  // Solving the equation for t gives us the solutions
  //
  //        1           ___________________________
  //   t = --- (-2ad ± √4(ad)^2 - 4(d^2 (a^2 - r^2))
  //        2
  //
  //        1            ________________________
  //     = --- (-2ad ± 2√(ad)^2 - d^2 (a^2 - r^2))
  //        2
  //              ________________________
  //     = -ad ± √(ad)^2 - d^2 (a^2 - r^2)
  // 
  // All real and nonnegative t's are then the sought parameters of intersection
  // for the ray formula.
  
  vector3 const a    = r.origin() - center_;
  double const  a_2  = norm_squared(a);
  double const  ad   = dot(a, r.direction().get());
  double const  ad_2 = ad * ad;
  double const  d_2  = norm_squared(r.direction().get());
  double const  r_2  = radius_ * radius_;
  double const  D    = ad_2 - d_2 * (a_2 - r_2);

  if (D < 0.0) return {};

  double const sqrt_D = std::sqrt(D);
  double t_1          = -ad + sqrt_D;
  double t_2          = -ad - sqrt_D;

  // Only consider nonnegative values for t_1, t_2, and make t_1 <= t_2.
  if (t_1 > t_2) std::swap(t_1, t_2);
  if (t_1 <= EPSILON) t_1 = t_2;
  if (t_1 <= EPSILON) return {};  // t_1 == 0 means we started out on the shape,
                                  // so no intersection.

  if (double_neq(t_1, t_2))
    return {t_1, t_2};
  else
    return {t_1};
}

auto sphere::normal_at(ray const& ray, double param) const -> unit<vector3> {
  vector3 const point = point_at(ray, param);

  // We have to decide the sign of the result based on whether the ray 
  // originates within the sphere or outside it, so that a hit "straight on"
  // the sphere (perpendicular to the surface) will always generate a normal
  // vector that points directly toward the origin.
  
  if (norm_squared(ray.origin() - center_) > radius_ * radius_) // If outside
    return point - center_;
  else
    return center_ - point;
}

plane::plane(vector3 const& point, vector3 const& u, vector3 const& v)
try
  : point_{point}
  , normal_{cross(u, v)} { }
catch (std::invalid_argument&) {
  // This must have been thrown from unit<>'s constructor because normal_
  // was initialized by a zero vector, which must be because u, v are colinear.
  throw std::invalid_argument{"plane: u, v linearly dependent"};
}

plane::plane(vector3 const& point, unit<vector3> const& normal)
  : point_{point}
  , normal_{normal} { }

auto plane::intersect(ray const& ray) const -> intersection_list {
  // Our plane is defined by the equation
  //
  //   <x - P, N> = 0,
  //
  // P being the given point on the plane, N the normal. Setting 
  // o := ray.origin(), d := ray.direction(), we can plug in the ray equation
  // and obtain
  //
  //   <o + td - P, N> = 0.
  //
  // Let us set a := o - P, to simplify the equation into
  //
  //        <a + td, N> = 0
  //   <a, N> + <td, N> = 0
  //            t<d, N> = -<a, N>
  //                       -<a, N>
  //                  t = ---------.
  //                        <d, N>
  //
  // The case when <d, N> = 0 corresponds to the situation where the ray is
  // parallel with the plane. We have no intersection in that case.
  //
  // Also notable is the case of <a, N> = 0, or <o - P, N> = 0 which happens
  // when the ray originates on the plane. Then two possibilities arise: 
  // Either 1) the ray goes somewhere out of the plane, and its origin
  // is the only common point with the plane; or 2) the ray is completely
  // embedded in the plane. In both cases we're going to report that no
  // intersection happens. In case 1) that is actually true (if we don't 
  // consider ray.origin() to be a part of the ray), in case 2) it is very 
  // false, but for ray tracing purposes it makes much more sense.
  //

  vector3 const a = ray.origin() - point_;
  double const aN = dot(a, normal_.get());
  double const dN = dot(ray.direction().get(), normal_.get());

  if (double_eq(dN, 0.0)) return {};  // Test for ray parallel to the plane.

  double const t  = -aN / dN;

  if (t > EPSILON)
    return {t};
  else
    return {};
}

auto plane::normal_at(ray const& ray, double) const -> unit<vector3> {
  // We want to get the normal pointing into the half-space containing 
  // o := ray.origin(). We have two half-spaces here, <o - P, N> < 0, and
  // <o - P, N> > 0, and two possible answers: N and -N. We know that the answer
  // N corresponds to the half-space <o - P, N> > 0 (the angle between the 
  // normal and the vector x - P is less than 90 degrees if it lies within the
  // normal vector's half-space).
  //
  // The question then is simple: Is <o - P, N> > 0? With x being the given
  // point on ray.

  double const d{dot(normalize(ray.origin() - point_), normal_.get())};

  if (d >= 0.0)  // The case d == 0.0 should be handled *somehow* as well.
    return normal_;
  else
    return -normal_.get();
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
  //                                                   specular_exponent
  //   I = diffuse ∙ cos(alpha) + specular ∙ cos(alpha),
  //
  // To add colours into the mix, we then multiply the light's colour with
  // its computed intensity.
  //
  // XXX: This should take distance to the light source into account as well.

  double const cos_alpha = dot(normal.get(), light_dir.get());

  assert(cos_alpha >= 0.0);

  color const diffuse_color{light.color() * diffuse_ * cos_alpha};
  color const specular_color{
    light.color() * specular_ * std::pow(cos_alpha, specular_exponent_)
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

