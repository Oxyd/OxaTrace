#ifndef OXATRACE_MATH_HPP
#define OXATRACE_MATH_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <boost/optional.hpp>

#include <initializer_list>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace oxatrace {
  
using random_eng = std::default_random_engine;

constexpr double PI{3.141592};
constexpr double EPSILON{1e-8};  // Minimal difference between numbers before
                                 // they are considered equal.

// Compare doubles for near-equality.
// Returns true iff the two numbers differ by less than EPSILON.
inline bool
double_eq(double a, double b) noexcept {
  return std::fabs(a - b) < EPSILON;
}

// Compare doubles for
// Returns the opposite of double_eq.
inline bool
double_neq(double a, double b) noexcept {
  return !double_eq(a, b);
}

// Round a floating-point number to an integer.
template <typename Integer>
Integer
round(double d) noexcept { return static_cast<Integer>(d + 0.5); }

inline bool
is_power2(unsigned n) {
  return n > 0 && (n & (n - 1)) == 0;
}

// Unit-length vector.
//
// Conversion from a vector to unit will automatically divide the vector by its
// magnitude.
//
// This is to help document the interface of various parts of the code and to
// prevent bugs that stem from using non-unit length vectors where a unit-length
// one is expected.
template <typename MatrixT>
class unit {
  static_assert(MatrixT::IsVectorAtCompileTime,
                "Only vectors can be unit-length.");

public:
  struct assume_norm { };

  unit(unit const&) = default;

  template <typename OtherMatrixT>
  unit(Eigen::MatrixBase<OtherMatrixT> const& other)
    : vector_{other.normalized()} { }

  template <typename OtherMatrixT>
  unit(assume_norm, Eigen::MatrixBase<OtherMatrixT> const& other)
    : vector_{other} {
    assert(double_eq(vector_.norm(), 1.0));
  }

  //template <typename T>
  //unit(T x) : unit(MatrixT{x}) { }

  template <typename T>
  unit(T x, T y) : unit(MatrixT{x, y}) { }

  template <typename T>
  unit(T x, T y, T z) : unit(MatrixT{x, y, z}) { }

  template <typename T>
  unit(T x, T y, T z, T w) : unit(MatrixT{x, y, z, w}) { }

  MatrixT get() const       { return vector_; }
  operator MatrixT () const { return vector_; }

private:
  MatrixT vector_;
};

using vector3 = Eigen::Vector3d;
using vector2 = Eigen::Vector2d;
using unit3   = unit<vector3>;
using unit2   = unit<vector2>;

// Get the cosine of the directed angle from v to u.
template <typename Base1, typename Base2>
auto
cos_angle(Eigen::MatrixBase<Base1> const& v,
          Eigen::MatrixBase<Base2> const& u) -> decltype(u.dot(v)) {
  return (u.dot(v)) / (u.norm() * v.norm());
}

template <typename Base, typename Unit>
auto
cos_angle(Eigen::MatrixBase<Base> const& u,
          unit<Unit> const& v) -> decltype(u.dot(v.get())) {
  return u.dot(v.get()) / u.norm();
}

template <typename Base, typename Unit>
auto
cos_angle(unit<Unit> const& u,
          Eigen::MatrixBase<Base> const& v) -> decltype(u.get().dot(v)) {
  return u.get().dot(v) / v.norm();
}

template <typename Unit1, typename Unit2>
auto
cos_angle(unit<Unit1> const& u, unit<Unit2> const& v)
  -> decltype(u.get().dot(v.get())) {
  return u.get().dot(v.get());
}

// Get any vector perpendicular to the given one.
unit3
get_any_orthogonal(unit3 const& v);

// Get the vector reflected off a surface, given the normal vector of the
// surface.
unit3
reflect(unit3 const& v, unit3 const& normal);

// Perturb a vector v by a random amount proportional to cosine lobe around v.
//
//             n + 1
// PDF: p(t) = ----- * cos(t)^n = probability that angle(result, v) = t.
//              2pi
unit3
cos_lobe_perturb(unit3 const& v, unsigned n, random_eng& prng);

// A ray is defined by its origin and direction; it is immutable.
//
// Direction isn't required to be a unit vector in order to allow for
// transformations of rays: A point on ray -- as given by point_at -- depends on
// the length of the direction vector.
class ray {
public:
  ray(vector3 const& origin, vector3 const& dir)
    : origin_{origin}
    , direction_{dir} { }

  vector3
  origin() const noexcept       { return origin_; }

  vector3
  direction() const noexcept    { return direction_; }

private:
  vector3 origin_;
  vector3 direction_;
};

std::ostream&
operator << (std::ostream& out, ray const& ray);

// Transform a ray by an affine matrix.
oxatrace::ray
transform(ray const& ray, Eigen::Affine3d const& tr);

// Get a point on ray.
//
// t must be non-negative.
vector3
point_at(ray const& r, double t);

// Lazily evaluated point on ray.
//
// This holds a ray and a parameter for point_at. It will lazily compute a point
// on the ray, and cache the result to avoid further re-evaluation.
class ray_point {
public:
  // param must be non-negative.
  ray_point(oxatrace::ray const& ray, double param);

  oxatrace::ray
  ray() const noexcept      { return ray_; }

  double
  param() const noexcept    { return param_; }

  // Compute the point on ray or fetch the cached one.
  vector3
  point() const;

private:
  oxatrace::ray ray_;
  double        param_;
  mutable boost::optional<vector3> point_;
};

// Two-dimensional rectangle in an unspecified space.
//
// Merely a container for four doubles.
class rectangle {
public:
  rectangle() { }

  // width and height must be positive.
  rectangle(double x, double y, double width, double height);

  double x() const noexcept { return x_; }
  double y() const noexcept { return y_; }
  void   x(double new_x) { x_ = new_x; }
  void   y(double new_y) { y_ = new_y; }

  vector2 top_left() const noexcept { return {x(), y()}; }

  double width() const noexcept  { return width_; }
  double height() const noexcept { return height_; }

  void   width(double new_width);
  void   height(double new_height);

private:
  double x_, y_;           // Coordinates of the top-left corner.
  double width_, height_;  // Dimensions of the rectangle.
};

// Construct a rectangle given coordinates of its centre point and its
// dimensions.
rectangle
rect_from_center(vector2 center, double width, double height);

// Get the point in the centre of a rectangle.
vector2
rect_center(rectangle r);

// Is one rectangle (non-strictly) inside another?
bool
within(rectangle inner, rectangle outer);

}  // namespace oxatrace

#endif
