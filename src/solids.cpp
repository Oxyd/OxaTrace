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
  // This sphere is defined by the equation ||x|| = 1. Let o := r.origin(), 
  // d := r.direction(), the ray is then described as 
  //
  //                       x = o + td, (forall t > 0).
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

unit3
sphere::normal_at(ray_point const& rp) const {
  return rp.point();
}

vector2
sphere::texture_at(ray_point const& rp) const {
  // We'll use the equations suggested at
  // http://en.wikipedia.org/wiki/UV_mapping :
  //
  //             atan(d.z / d.x)
  //   u = 0.5 + ---------------
  //                  2 pi
  //
  //             asin(d.y)
  //   v = 0.5 - ---------
  //                pi
  
  unit3 const d = -normal_at(rp);
  double const u = 0.5 + atan2(d.z(), d.x()) / (2 * PI);
  double const v = 0.5 - asin(d.y()) / PI;

  assert(0.0 <= u && u <= 1.0);
  assert(0.0 <= v && v <= 1.0);

  return {u, v};
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

unit3
plane::normal_at(ray_point const& rp) const {
  // We need to consider the ray origin here in order to determine the "sign"
  // of the result: Plane can be viewed both from the front and from the behind,
  // and no way is "inside" or "outside".
  
  if (rp.ray().origin().z() > 0.0)  // Hit from the front.
    return -vector3::UnitZ();
  else
    return vector3::UnitZ();
}

vector2
plane::texture_at(ray_point const& rp) const {
  // Planes are infinite, so we'll just pretend we're texturing a square, and
  // let the texture repeat across the entire plane.
  //
  // So get the point coordinates into [0, 1]^2 by taking just the fractional
  // part of the point coordinates, and simply return that.

  double dummy;

  double u = std::modf(rp.point().x(), &dummy);
  double v = std::modf(rp.point().y(), &dummy);

  if (u < 0.0) u += 1.0;  // Not abs, to prevent weird things as we cross 0.
  if (v < 0.0) v += 1.0;

  assert(0 <= u && u <= 1.0);
  assert(0 <= v && v <= 1.0);

  return {u, v};
}

checkerboard::checkerboard(hdr_color a, hdr_color b, unsigned num)
  : color_a{a}
  , color_b{b}
  , divisor_{1.0 / num}
{ }

hdr_color
checkerboard::get(double u, double v) const {
  bool const a = (unsigned) (u / divisor_) % 2;
  bool const b = (unsigned) (v / divisor_) % 2;
  return a != b ? color_b : color_a;
}

material::material(hdr_color const& ambient, double diffuse, double specular,
                   unsigned specular_exponent, double reflectance)
  : ambient_{ambient}
  , diffuse_{diffuse}
  , specular_{specular} 
  , specular_exponent_{specular_exponent}
  , reflectance_{reflectance}
{
  if (diffuse < 0.0 || diffuse > 1.0)
    throw std::invalid_argument{"material: Invalid diffuse coefficient."};
  if (specular < 0.0 || specular > 1.0)
    throw std::invalid_argument{"material: Invalid specular coefficient."};
  if (reflectance_ < 0.0 || reflectance_ > 1.0)
    throw std::invalid_argument{"material: Invalid reflectance value."};
}

solid::solid(std::shared_ptr<oxatrace::shape> const& s, oxatrace::material mat,
             std::shared_ptr<texture> const& texture)
  : shape_{s}
  , texture_{texture}
  , material_{mat}
  , world_to_object_{Eigen::Affine3d::Identity()}
  , object_to_world_{Eigen::Affine3d::Identity()} { }

material const&
solid::material() const noexcept {
  return material_;
}

shape::intersection_list
solid::intersect(ray const& ray) const {
  oxatrace::ray const object_ray = oxatrace::transform(ray, world_to_object_);
  return shape_->intersect(object_ray);
}

unit3
solid::normal_at(ray_point const& rp) const {
  vector3 const local_normal = shape_->normal_at(local_ray_point(rp));
  return object_to_world_.linear().transpose() * local_normal;
}

hdr_color
solid::texture_at(ray_point const& rp) const {
  if (texture_) {
    vector2 const uv = shape_->texture_at(local_ray_point(rp));
    return texture_->get(uv[0], uv[1]);
  } else {
    return material_.base_color();
  }
}

void
solid::set_texture(std::shared_ptr<texture> const& new_texture) {
  texture_ = new_texture;
}

solid&
solid::translate(vector3 const& tr) {
  object_to_world_.pretranslate(tr);
  world_to_object_.translate(-tr);
  return *this;
}

solid&
solid::scale(double coef) {
  if (coef < EPSILON)
    throw std::invalid_argument{"solid::scale: coef <= 0"};

  object_to_world_.prescale(coef);
  world_to_object_.scale(1. / coef);
  return *this;
}

solid&
solid::scale(double x, double y, double z) {
  if (x < EPSILON || y < EPSILON || z < EPSILON)
    throw std::invalid_argument{"solid::scale: x, y, or z <= 0.0"};
  
  vector3 const scale_vec{x, y, z};
  vector3 const scale_vec_rec{1. / x, 1. / y, 1. / z};

  object_to_world_.prescale(scale_vec);
  world_to_object_.scale(scale_vec_rec);
  
  return *this;
}

solid&
solid::rotate(Eigen::AngleAxisd const& rot) {
  object_to_world_.prerotate(rot);
  world_to_object_.rotate(rot.inverse());
  return *this;
}

solid&
solid::transform(Eigen::Affine3d const& tr, Eigen::Affine3d const& inverse) {
  assert((tr * inverse).isApprox(Eigen::Affine3d::Identity()));

  object_to_world_ = tr * object_to_world_;
  world_to_object_ = world_to_object_ * inverse;

  return *this;
}

solid&
solid::transform(Eigen::Affine3d const& tr) {
  Eigen::Affine3d inverse = tr.inverse();
  return transform(tr, inverse);
}

ray_point
solid::local_ray_point(ray_point const& global) const {
  return {oxatrace::transform(global.ray(), world_to_object_), global.param()};
}
