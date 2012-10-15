#include "math.hpp"

namespace oxatrace {

auto point_at(ray r, double t) -> vector3 {
  if (t >= 0.0)
    return r.origin() + t * r.direction();
  else
    throw std::logic_error("point_at: t < 0.0");
}

}

// vim:colorcolumn=80
