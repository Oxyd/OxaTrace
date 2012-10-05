#include "color.hpp"

#include <algorithm>

using namespace oxatrace;

// Clamp a value into an interval.
static
auto clamp(double x, double lower, double upper) -> double {
  return std::max(std::min(x, upper), lower);
}

channel::channel(double value)
  : value_{clamp(value, MIN, MAX)} { }

auto channel::operator = (double d) -> channel& {
  set(d);
  return *this;
}

void channel::set(double value)     { value_ = clamp(value, MIN, MAX);}
channel::operator double () const   { return get(); }
auto channel::get() const -> double { return value_; }

auto channel::operator += (channel other) -> channel& {
  set(value_ + other.value_);
  return *this;
}

auto channel::operator -= (channel other) -> channel& {
  set(value_ - other.value_);
  return *this;
}

auto channel::operator *= (channel other) -> channel& { 
  set(value_ * other.value_);
  return *this;
}

auto channel::operator /= (channel other) -> channel& {
  set(value_ / other.value_);
  return *this;
}

color::color() { }
color::color(channel r, channel g, channel b)
  : channels_{{r, g, b}} { }

auto color::operator += (color other) -> color& {
  r() += other.r();
  g() += other.g();
  b() += other.b();
  return *this;
}

auto color::operator -= (color other) -> color& {
  r() -= other.r();
  g() -= other.g();
  b() -= other.b();
  return *this;
}

auto color::operator *= (color other) -> color& {
  r() *= other.r();
  g() *= other.g();
  b() *= other.b();
  return *this;
}

auto color::operator /= (color other) -> color& {
  r() /= other.r();
  g() /= other.g();
  b() /= other.b();
  return *this;
}

