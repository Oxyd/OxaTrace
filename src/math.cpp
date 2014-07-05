#include "math.hpp"

#include <stdexcept>

using namespace oxatrace;

unit3
oxatrace::get_any_orthogonal(unit3 const& v) {
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
  return v - 2.0 * v.dot(normal) * normal;
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
  if (t >= 0.0)
    return r.origin() + t * r.direction();
  else
    throw std::logic_error("point_at: t < 0.0");
}

ray_point::ray_point(oxatrace::ray const& ray, double param)
  : ray_{ray}
  , param_{param} {
  if (param < 0.0) throw std::logic_error{"ray_point: param < 0.0"};
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
  if (width_ <= 0.0 || height_ <= 0.0)
    throw std::logic_error{"rectangle::rectangle: nonpositive dimension"};
}

void
oxatrace::rectangle::width(double new_width) {
  if (new_width > 0.0)
    width_ = new_width;
  else
    throw std::logic_error{"rectangle::width: nonpositive dimension"};
}

void
oxatrace::rectangle::height(double new_height) {
  if (new_height > 0.0)
    height_ = new_height;
  else
    throw std::logic_error{"rectangle::height: nonpositive dimension"};
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
