#ifndef OXATRACE_MATH_HPP
#define OXATRACE_MATH_HPP

#include <Eigen/Core>

#include <boost/optional.hpp>

#include <utility>

namespace oxatrace {

// Numbers --------------------------------------------------------------------

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

// Vectors & Matrices ---------------------------------------------------------

template <typename MatrixT>
class unit : public MatrixT {
  static_assert(MatrixT::IsVectorAtCompileTime,
                "Only vectors can be unit-length.");

public:
  template <typename OtherDerived>
  unit(Eigen::MatrixBase<OtherDerived> const& other)
    : MatrixT(other.normalized()) { }

  template <typename OtherMatrixT>
  unit(unit<OtherMatrixT> const& other)
    : MatrixT(other) { }

  template <typename... Elems>
  unit(Elems... elems)
    : MatrixT(MatrixT(elems...).normalized()) { }

  unit(unit const&) = default;

  auto norm() -> decltype(std::declval<MatrixT>().norm()) {
    return 1.0;
  }

  auto squaredNorm() -> decltype(std::declval<MatrixT>().squaredNorm()) {
    return 1.0;
  }
};

using vector3 = Eigen::Vector3d;
using unit3   = unit<vector3>;

// Get the cosine of the directed angle from v to u.
template <typename Base1, typename Base2>
auto cos_angle(
  Eigen::MatrixBase<Base1> const& v,
  Eigen::MatrixBase<Base2> const& u,
  typename std::enable_if<
    Eigen::MatrixBase<Base1>::IsVectorAtCompileTime &&
    Eigen::MatrixBase<Base2>::IsVectorAtCompileTime &&
    static_cast<std::size_t>(Eigen::MatrixBase<Base1>::SizeAtCompileTime) ==
      static_cast<std::size_t>(Eigen::MatrixBase<Base2>::SizeAtCompileTime)
  >::type* = 0
) -> decltype(u.dot(v)) 
{ return (u.dot(v)) / (u.norm() * v.norm()); }

// Rays -----------------------------------------------------------------------

// A ray is defined by its origin and direction. Rays are immutable.
class ray {
public:
  // Construction...
  ray(vector3 const& origin, unit3 const& dir)
    : origin_{origin}
    , direction_{dir} { }

  // Observers...
  auto origin() const noexcept -> vector3 {
    return origin_; 
  }

  auto direction() const noexcept -> unit3 {
    return direction_; 
  }

private:
  vector3 origin_;
  unit3   direction_;
};

auto operator << (std::ostream& out, ray const& ray) -> std::ostream&;

// Given a parametric ray r(t), compute r(t).
// Throws:
//   -- std::logic_error: When t is negative.
auto point_at(ray const& r, double t) -> vector3;

// Contains both a ray and a point on this ray. The point is computed lazily
// and cached to avoid its re-computation.
class ray_point {
public:
  // Construction...
  // Throws:
  //   -- std::logic_error: When param is negative.
  ray_point(oxatrace::ray const& ray, double param);

  // Observers...
  auto ray() const noexcept   -> oxatrace::ray { return ray_; }
  auto param() const noexcept -> double        { return param_; }
  auto point() const          -> vector3;

private:
  oxatrace::ray ray_;
  double        param_;
  mutable boost::optional<vector3> point_;
};


}

#endif

