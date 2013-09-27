#include "lights.hpp"

using namespace oxatrace;

point_light::point_light(vector3 const& p, hdr_color const& c)
  : position_(p)
  , color_(c) { }

vector3
point_light::get_source() const {
  return position_;
}

hdr_color
point_light::color() const {
  return color_;
}

