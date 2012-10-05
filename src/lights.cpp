#include "lights.hpp"

using namespace oxatrace;

point_light::point_light(vector3 const& p, oxatrace::color const& c)
  : position_(p)
  , color_(c) { }

auto point_light::get_source() const -> vector3    { return position_; }
auto point_light::color() const -> oxatrace::color { return color_; }

