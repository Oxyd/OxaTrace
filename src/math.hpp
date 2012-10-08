#ifndef OXATRACE_MATH_HPP
#define OXATRACE_MATH_HPP

#include <boost/operators.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <utility>

namespace oxatrace {

constexpr double PI{3.141592};
constexpr double EPSILON{1e-8};

// Return true iff the two given doubles differ by less than EPSILON.
inline auto double_eq(double a, double b) -> bool {
  return std::fabs(a - b) < EPSILON;
}

// Return true iff the two given doubles differ by at least EPSILON.
inline auto double_neq(double a, double b) -> bool {
  return !double_eq(a, b);
}
  
// Vectors & Rays --------------------------------------------------------------

// N-dimensional arithmetic vector.
template <std::size_t N>
class vector 
  : boost::additive<vector<N>>
  , boost::multipliable2<vector<N>, double>
  , boost::dividable<vector<N>, double>
  , boost::equality_comparable<vector<N>> {
  static_assert(N >= 1, "Can't create 0-dimensional vector.");
  using component_list = std::array<double, N>;
public:
  // Types...
  using iterator       = typename component_list::iterator;
  using const_iterator = typename component_list::const_iterator;

  // Constants...
  static constexpr double COMPONENTS{N};

  // Construction...
  vector() { }  // Uninitialized vector.
  template <typename... Components>
  vector(Components... components)  // sizeof...(Components) must equal N.
    : components_{{components...}} { 
    static_assert(sizeof...(Components) == N, "Wrong number of components.");
  }

  static auto zero()   -> vector { 
    vector ret;
    std::fill(ret.components_.begin(), ret.components_.end(), 0.0);
    return ret;
  }

  static auto unit_x() -> vector {  // N must be >= 1.
    vector ret{zero()};
    ret.x() = 1.0;
    return ret;
  }

  static auto unit_y() -> vector {  // N must be >= 2.
    vector ret{zero()};
    ret.y() = 1.0;
    return ret;
  }

  static auto unit_z() -> vector {  // N must be >= 3.
    vector ret{zero()};
    ret.z() = 1.0;
    return ret;
  }

  static auto unit_w() -> vector {  // N must be >= 4.
    vector ret{zero()};
    ret.w() = 1.0;
    return ret;
  }

  // Component access...
  auto x() -> double& {
    static_assert(N >= 1, "No x component in this vector");
    return components_[0];
  }

  auto y() -> double& {
    static_assert(N >= 2, "No y component in this vector");
    return components_[1];
  }

  auto z() -> double& {
    static_assert(N >= 3, "No z component in this vector");
    return components_[2];
  }

  auto w() -> double& {
    static_assert(N >= 4, "No w component in this vector");
    return components_[3];
  }

  auto x() const -> double { return const_cast<vector*>(this)->x(); }
  auto y() const -> double { return const_cast<vector*>(this)->y(); }
  auto z() const -> double { return const_cast<vector*>(this)->z(); }
  auto w() const -> double { return const_cast<vector*>(this)->w(); }

  auto operator [] (std::size_t index) -> double& { return components_[index]; }
  auto operator [] (std::size_t index) const -> double {
    return components_[index];
  }

  auto begin()        -> iterator       { return components_.begin(); }
  auto end()          -> iterator       { return components_.end(); }
  auto begin() const  -> const_iterator { return cbegin(); }
  auto end() const    -> const_iterator { return cend(); }
  auto cbegin() const -> const_iterator { return components_.begin(); }
  auto cend() const   -> const_iterator { return components_.end(); }

  // Vector space operators...
  auto operator += (vector<N> const& other) -> vector<N>& {
    std::transform(begin(), end(), other.begin(),
                   begin(), std::plus<double>());
    return *this;
  }

  auto operator -= (vector<N> const& other) -> vector<N>& {
    std::transform(begin(), end(), other.begin(), 
                   begin(), std::minus<double>());
    return *this;
  }

  auto operator *= (double scalar) -> vector<N>& {
    std::transform(begin(), end(), begin(),
                   [scalar] (double d) { return d * scalar; });
    return *this;
  }

  auto operator /= (double scalar) -> vector<N>& {
    std::transform(begin(), end(), begin(),
                   [scalar] (double d) { return d / scalar; });
    return *this;
  }
  
private:
  component_list components_;
};

using vector3 = vector<3>;
using vector4 = vector<4>;

// Equality comparison.
template <std::size_t N>
auto operator == (vector<N> const& u, vector<N> const& v) -> bool {
  return std::equal(u.begin(), u.end(), v.begin(), double_eq);
}

// Get the additive inverse of a vector.
template <std::size_t N>
auto operator - (vector<N> const& v) -> vector<N> {
  vector<N> ret;
  std::transform(v.begin(), v.end(), ret.begin(),
                 [](double d) { return -d; });
  return ret;
}

// Get the dot-product of the two given vectors.
template <std::size_t N>
auto dot(vector<N> const& u, vector<N> const& v) -> double {
  return std::inner_product(u.begin(), u.end(), v.begin(), 0.0);
}

// Get the cross-product of two 3-dimensional vectors.
inline
auto cross(vector3 const& u, vector3 const& v) -> vector3 {
  return {u.y() * v.z() - u.z() * v.y(),
          u.z() * v.x() - u.x() * v.z(),
          u.x() * v.y() - u.y() * v.x()};
}

// Streaming support.
template <std::size_t N>
auto operator << (std::ostream& out, vector<N> const& v) -> std::ostream& {
  out << "(";
  std::copy(v.begin(), std::prev(v.end()),
            std::ostream_iterator<double>(out, ", "));
  return out << *std::prev(v.end()) << ")^T";
}

// Get the square of the Euclidean (L2) norm of a vector. That is the same
// as <v, v>.
template <std::size_t N>
auto norm_squared(vector<N> const& v) -> double {
  return dot(v, v);
}

// Get the Euclidean (L2) norm of the given vector.
template <std::size_t N>
auto norm(vector<N> const& v) -> double {
  return std::sqrt(norm_squared(v));
}

// Is the given vector a zero-length one?
template <std::size_t N>
auto zero(vector<N> const& v) -> bool { return double_eq(norm(v), 0.0); }

// Get the vector that is colinear with the given one, but has norm == 1.
// Throws:
//   -- std::invalid_argument: if zero(v).
template <std::size_t N>
auto normalize(vector<N> const& v) -> vector<N> {
  double const L = norm(v);
  if (double_neq(L, 0.0))
    return v / L;
  else
    throw std::invalid_argument{"normalize: Given a zero vector"};
}

// Unit-length vectors are those with .norm() == 1.0. They must be non-zero.
template <typename Vector>
class unit : boost::equality_comparable<unit<Vector>, Vector> {
public:
  // Construction...
  unit(Vector v)
    : v_{normalize(v)} { }
  
  unit(unit<Vector> const& other) noexcept
    : v_{other.v_} { }

  // Forwarding ctor to allow for creation of unit<V> with a simple {x, y, z}.
  template <typename... Args>
  unit(Args... args) 
    : v_{normalize(vector<sizeof...(Args)>{args...})} { }
    
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
