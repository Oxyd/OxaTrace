#include "math.hpp"

using namespace oxatrace;

auto oxatrace::operator << (std::ostream& out, ray const& ray) 
  -> std::ostream& {
  return out << "ray{origin = " << ray.origin()
             << ", direction = " << ray.direction().get()
             << "}";
}

auto oxatrace::point_at(ray const& r, double t) -> vector3 {
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

auto ray_point::point() const -> vector3 {
  if (!point_)
    point_ = point_at(ray_, param_);
  return *point_;
}

