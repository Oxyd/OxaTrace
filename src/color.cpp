#include "color.hpp"

#include <algorithm>

using namespace oxatrace;

// Clamp a value into an interval.
static double
clamp(double x, double lower, double upper) {
  return std::max(std::min(x, upper), lower);
}

channel::channel(double value)
  : value_{clamp(value, MIN, MAX)} { }

channel&
channel::operator = (double d) {
  set(d);
  return *this;
}

void channel::set(double value)     { value_ = clamp(value, MIN, MAX);}
channel::operator double () const   { return get(); }
auto channel::get() const -> double { return value_; }

channel&
channel::operator += (channel other) {
  set(value_ + other.value_);
  return *this;
}

channel&
channel::operator -= (channel other) {
  set(value_ - other.value_);
  return *this;
}

channel&
channel::operator *= (channel other) {
  set(value_ * other.value_);
  return *this;
}

channel&
channel::operator /= (channel other) {
  set(value_ / other.value_);
  return *this;
}

color::color() { }
color::color(channel r, channel g, channel b)
  : channels_{{r, g, b}} { }

color&
color::operator += (color other) {
  r() += other.r();
  g() += other.g();
  b() += other.b();
  return *this;
}

color&
color::operator -= (color other) {
  r() -= other.r();
  g() -= other.g();
  b() -= other.b();
  return *this;
}

color&
color::operator *= (color other) {
  r() *= other.r();
  g() *= other.g();
  b() *= other.b();
  return *this;
}

color&
color::operator /= (color other) {
  r() /= other.r();
  g() /= other.g();
  b() /= other.b();
  return *this;
}

color&
color::operator *= (double d) {
  r() *= d;
  g() *= d;
  b() *= d;
  return *this;
}

color&
color::operator /= (double d) {
  r() /= d;
  g() /= d;
  b() /= d;
  return *this;
}

