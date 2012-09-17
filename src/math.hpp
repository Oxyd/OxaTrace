#ifndef OXATRACE_MATH_HPP
#define OXATRACE_MATH_HPP

#include <Eigen/Core>

#include <boost/operators.hpp>

#include <stdexcept>
#include <algorithm>
#include <utility>

namespace oxatrace {

double const EPSILON{0.000001};
  
// Vectors & Rays --------------------------------------------------------------

using vector3 = Eigen::Vector3d;
using vector4 = Eigen::Vector4d;

// Unit-length vectors are those with .norm() == 1.0. They must be non-zero.
template <typename Vector>
class unit : boost::equality_comparable<unit<Vector>, Vector> {
public:
  // Construction...
  unit(Vector v)
    : v_{normalize(v)} { }
  
  unit(unit<Vector> const& other) noexcept
    : v_(other.v_) { }
    
  // Modifiers...
  unit& operator = (Vector v) {
    v_ = normalize(v); 
    return *this;
  }
  
  unit& operator = (unit<Vector> other) noexcept {
    other.swap(*this);
    return *this;
  }
  
  void swap(unit<Vector>& other) noexcept { std::swap(v_, other.v_); }

  // Observers...
  auto get() const noexcept -> Vector { return v_; }
  operator Vector () const noexcept   { return get(); }

private:
  Vector v_;
  
  auto normalize(Vector v) -> Vector { 
    if (!v.isZero()) 
      return v.normalized();
    else
      throw std::logic_error("Attempted to initialize unit<> from a zero vector");
  }
};

// Comparison of unit<Vector> with Vector.
template <typename Vector>
bool operator == (unit<Vector> const& u, Vector const& v) {
  return u.get() == v;
}

// A ray is defined by its origin and direction. Rays are immutable.
class ray {
public:
  // Construction...
  ray(vector3 origin, unit<vector3> dir)
    : origin_{origin}
    , direction_{dir} { }

  // Observers...
  auto origin()    const noexcept -> vector3        { return origin_; }
  auto direction() const noexcept -> unit<vector3>  { return direction_; }

private:
  vector3        origin_;
  unit<vector3>  direction_;
};

// Given a parametric ray r(t), compute r(t).
// Throws:
//   -- std::logic_error: When t is negative.
auto point_at(ray r, double t) -> vector3;

}

#endif

// vim:colorcolumn=80
