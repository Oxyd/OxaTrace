#include "solids.hpp"

#include "color.hpp"
#include "lights.hpp"

#include <Eigen/Geometry>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <iostream>

using namespace oxatrace;

auto sphere::intersect(ray const& ray) const -> intersection_list {
  // This sphere is defined by the equation ||x|| = 1. Let o := r.origin(), d:=
  // r.direction(), the ray is then described as x = o + td, (forall t > 0).
  //
  // Substituting the ray equation into the sphere definition gives
  //
  //                            ||o + td|| = 1.
  //
  // Since both sides are non-negative and ||.|| denotes the Euclidean norm, we
  // have
  //
  //                                        ||o + td|| = 1
  //                                      ||o + td||^2 = 1
  //                                  <o + td, o + td> = 1
  //                    <o, o> + 2t<o, d> + t^2 <d, d> = 1
  //              t^2 ||d||^2 + 2t<o, d> + ||o||^2 - 1 = 0,
  //
  // a quadratic equation in t.
  //
  // Solving the equation for t gives us the solutions
  //
  //                 1                   ___________________________________
  //        t = ----------- (-2<o, d> ± √4<o, d>^2 - 4 ||d||^2 (||o||^2 - 1))
  //             2 ||d||^2
  //
  //                 1                    ________________________________
  //          = ----------- (-2<o, d> ± 2√<o, d>^2 - ||d||^2 (||o||^2 - 1))
  //             2 ||d||^2
  //
  //                1                 ________________________________
  //          = --------- (-<o, d> ± √<o, d>^2 - ||d||^2 (||o||^2 - 1))
  //             ||d||^2
  //
  // All real and nonnegative t's are then the sought parameters of intersection
  // for the ray formula.

  double const od   = ray.origin().dot(ray.direction());
  double const od_2 = od * od;
  double const d_2  = ray.direction().squaredNorm();
  double const o_2  = ray.origin().squaredNorm();
  double const D    = od_2 - d_2 * (o_2 - 1);

  if (D < 0.0) return {};

  double const sqrt_D = std::sqrt(D);
  double const t_1    = (-od - sqrt_D) / d_2;
  double const t_2    = (-od + sqrt_D) / d_2;

  assert(t_1 <= t_2);
  assert(t_1 <= EPSILON || double_eq(point_at(ray, t_1).norm(), 1.0));
  assert(t_2 <= EPSILON || double_eq(point_at(ray, t_2).norm(), 1.0));

  // Ignore intersections that are too close to zero. The origin itself isn't
  // to be considered a part of the ray, and values close to it may result
  // as a consequence of floating-point arithmetic.

  if (t_1 > EPSILON)      return {t_1, t_2};
  else if (t_2 > EPSILON) return {t_2};
  else                    return {};
}

auto sphere::normal_at(ray_point const& rp) const -> unit3 {
  return rp.point();
}

auto plane::intersect(ray const& ray) const -> intersection_list {
  // Since this is an xy plane, we're solving the equation o_z + td_z = 0,
  // where o_z and d_z are the z components of the ray origin and direction
  // respectively. The solution is t = -o_z / d_z.
  //
  // The case of d_z = 0 corresponds to the case of a ray parallel to the plane
  // or embedded in the plane. In both cases we will report that there is no
  // intersection.

  if (double_eq(ray.direction().z(), 0.0)) return {};

  double const t = -ray.origin().z() / ray.direction().z();
  if (t > EPSILON) return {t};
  else             return {};
}

auto plane::normal_at(ray_point const& rp) const -> unit3 {
  // We need to consider the ray origin here in order to determine the "sign"
  // of the result: Plane can be viewed both from the front and from the behind,
  // and no way is "inside" or "outside".
  
  if (rp.ray().origin().z() > 0.0)  // Hit from the front.
    return -vector3::UnitZ();
  else
    return vector3::UnitZ();
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
  //   I = diffuse * cos(alpha) + specular * cos(alpha),
  //
  // To add colours into the mix, we then multiply the light's colour with
  // its computed intensity.
  //
  // XXX: This should take distance to the light source into account as well.

  double const cos_alpha = cos_angle(normal, light_dir);

  if (cos_alpha <= 0.0) return base_color;

  color const diffuse_color{light.color() * diffuse_ * cos_alpha};
  color const specular_color{
    light.color() * specular_ * std::pow(cos_alpha, specular_exponent_)
  };

  return base_color + diffuse_color + specular_color;
}

solid::solid(std::shared_ptr<oxatrace::shape> const& s, oxatrace::material mat)
  : shape_{s}
  , material_{mat}
  , world_to_object_{Eigen::Affine3d::Identity()}
  , object_to_world_{Eigen::Affine3d::Identity()} { }

auto solid::material() const noexcept -> oxatrace::material const& {
  return material_;
}

auto solid::intersect(ray const& ray) const -> shape::intersection_list {
  oxatrace::ray const object_ray = oxatrace::transform(ray, world_to_object_);
  return shape_->intersect(object_ray);
}

auto solid::normal_at(ray_point const& rp) const -> unit3 {
  vector3 const local_normal = shape_->normal_at(
    {oxatrace::transform(rp.ray(), world_to_object_), rp.param()}
  );
  return object_to_world_.linear().transpose() * local_normal;
}

auto solid::translate(vector3 const& tr) -> solid& {
  object_to_world_.pretranslate(tr);
  world_to_object_.translate(-tr);
  return *this;
}

auto solid::scale(double coef) -> solid& {
  if (coef < EPSILON)
    throw std::invalid_argument{"solid::scale: coef <= 0"};

  object_to_world_.prescale(coef);
  world_to_object_.scale(1. / coef);
  return *this;
}

auto solid::scale(double x, double y, double z) -> solid& {
  if (x < EPSILON || y < EPSILON || z < EPSILON)
    throw std::invalid_argument{"solid::scale: x, y, or z <= 0.0"};
  
  vector3 const scale_vec{x, y, z};
  vector3 const scale_vec_rec{1. / x, 1. / y, 1. / z};

  object_to_world_.prescale(scale_vec);
  world_to_object_.scale(scale_vec_rec);
  
  return *this;
}

auto solid::rotate(Eigen::AngleAxisd const& rot) -> solid& {
  object_to_world_.prerotate(rot);
  world_to_object_.rotate(rot.inverse());
  return *this;
}

auto solid::transform(Eigen::Affine3d const& tr,
                      Eigen::Affine3d const& inverse) -> solid& {
  assert((tr * inverse).isApprox(Eigen::Affine3d::Identity()));

  object_to_world_ = tr * object_to_world_;
  world_to_object_ = world_to_object_ * inverse;

  return *this;
}

auto solid::transform(Eigen::Affine3d const& tr) -> solid& {
  Eigen::Affine3d inverse = tr.inverse();
  return transform(tr, inverse);
}
