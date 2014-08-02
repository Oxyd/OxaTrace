#include "math.hpp"

#include <cmath>
#include <stdexcept>

using namespace oxatrace;

unit3
oxatrace::get_any_orthogonal(unit3 const& input) {
  // If u is the solution, it holds that <v, u> = 0, or
  // 
  //                v_x u_x + v_y u_y + v_z u_z = 0.
  //
  // Let u_x = u_y = 1, then u_z is given as
  //
  //                           1
  //                    u_z = --- (-v_x - v_y).
  //                          v_z
  //
  // That assumes v_z =/= 0. If that isn't the case, one can set two components
  // of u to 1 and solve for the third one. That is always possible since v
  // can't be a zero vector.
  //
  // To enhance numerical stability, we will choose the largest (in absolute
  // value) component of v, fix the other components of u and solve for the
  // remaining one.

  vector3 const v = input.get();

  double const x = std::abs(v.x());
  double const y = std::abs(v.y());
  double const z = std::abs(v.z());

  if (x >= y && x >= z)
    return {(-v.y() - v.z()) / v.x(), 1.0, 1.0};
  else if (y >= x && y >= z)
    return {1.0, (-v.x() - v.z()) / v.y(), 1.0};
  else
    return {1.0, 1.0, (-v.x() - v.y()) / v.z()};
}

unit3
oxatrace::reflect(unit3 const& v, unit3 const& normal) {
  return v.get() - 2.0 * v.get().dot(normal.get()) * normal.get();
}

unit3
oxatrace::cos_lobe_perturb(unit3 const& v, unsigned n, random_eng& prng) {
  // We'll use the formulas from Philip Dutré's Total Compendium[1] to generate
  // a random vector on a hemisphere.
  //
  // Dutré's formula assumes the hemisphere is positioned in the origin, and 
  // bulging upwards in the z direction. We'll need to transform the result
  // so that the direction of the hemisphere is given by the input z vector.
  //
  // To do that, we'll estabilish a coordinate system, x, y, z; z being the 
  // input parameter. x and y are then any two vectors so that x, y, z forms
  // an orthonormal basis. 
  // 
  // [1] http://people.cs.kuleuven.be/~philip.dutre/GI/TotalCompendium.pdf
  
  // Our orthonormal basis.
  vector3 const z = v.get();
  vector3 const x = get_any_orthogonal(z);
  vector3 const y = x.cross(z);
  
  std::uniform_real_distribution<> phi_distrib(0, 2 * PI);
  std::uniform_real_distribution<> u_distrib;
  
  double const phi = phi_distrib(prng);
  double const r   = u_distrib(prng);  // r_2 in [1].
  
  double const p = std::pow(r, 2.0 / (n + 1.0));
  double const q = std::sqrt(1.0 - p);
  
  return 
    x * std::cos(phi) * q
    + y * std::sin(phi) * q
    + z * std::pow(r, 1.0 / (n + 1.0));
}

std::ostream&
oxatrace::operator << (std::ostream& out, ray const& ray) {
  return out << "ray{origin =\n" << ray.origin()
             << "\ndirection = " << ray.direction()
             << "\n}";
}

oxatrace::ray
oxatrace::transform(ray const& ray, Eigen::Affine3d const& tr)
{ return {tr * ray.origin().homogeneous(), tr.linear() * ray.direction()}; }

Eigen::Vector3d
oxatrace::point_at(ray const& r, double t) {
  assert(t >= 0.0);
  return r.origin() + t * r.direction();
}

ray_point::ray_point(oxatrace::ray const& ray, double param)
  : ray_{ray}
  , param_{param}
{
  assert(param >= 0.0);
}

Eigen::Vector3d
ray_point::point() const {
  if (!point_)
    point_ = point_at(ray_, param_);
  return *point_;
}

oxatrace::rectangle::rectangle(double x, double y, double width, double height)
  : x_{x}
  , y_{y}
  , width_{width}
  , height_{height}
{
  assert(width > 0.0 && height > 0.0);
}

void
oxatrace::rectangle::width(double new_width) {
  assert(new_width > 0.0);
  width_ = new_width;
}

void
oxatrace::rectangle::height(double new_height) {
  assert(new_height > 0.0);
  height_ = new_height;
}

oxatrace::rectangle
oxatrace::rect_from_center(vector2 center, double width, double height) {
  return {
    center[0] - width / 2, center[1] - height / 2,
    width, height
  };
}

oxatrace::vector2
oxatrace::rect_center(rectangle r) {
  return {r.x() + r.width() / 2.0, r.y() + r.height() / 2.0};
}

bool
oxatrace::within(rectangle inner, rectangle outer) {
  return
    inner.x() - outer.x() >= -EPSILON
    && inner.y() - outer.y() >= -EPSILON
    && inner.x() + inner.width() - (outer.x() + outer.width()) <= EPSILON
    && inner.y() + inner.height() - (outer.y() + outer.height()) <= EPSILON
    ;
}
