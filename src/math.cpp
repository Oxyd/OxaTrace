#include "math.hpp"

#include <stdexcept>

using namespace oxatrace;

auto oxatrace::operator << (std::ostream& out, ray const& ray) 
  -> std::ostream& {
  return out << "ray{origin = " << ray.origin()
             << ", direction = " << ray.direction()
             << "}";
}

auto oxatrace::point_at(ray const& r, double t) -> Eigen::Vector3d {
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

auto ray_point::point() const -> Eigen::Vector3d {
  if (!point_)
    point_ = point_at(ray_, param_);
  return *point_;
}

