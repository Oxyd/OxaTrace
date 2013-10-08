/// \file math.hpp

#ifndef OXATRACE_MATH_HPP
#define OXATRACE_MATH_HPP

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <boost/optional.hpp>

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace oxatrace {

/// \defgroup math Maths

/// \defgroup numbers Numbers
/// \ingroup math

/// \name Constants
///@{

/// \ingroup numbers

constexpr double PI{3.141592};
constexpr double EPSILON{1e-8};  ///< Minimal difference between numbers before
                                 ///< they are considered equal.

///@}

/// \name Operations
///@{

/// \ingroup numbers

/// \brief Compare doubles for equality.
/// \return true iff the two numbers differ by less than EPSILON.
inline bool
double_eq(double a, double b) noexcept {
  return std::fabs(a - b) < EPSILON;
}

/// \brief Compare doubles for inequality.
/// \return The opposite of double_eq.
inline bool
double_neq(double a, double b) noexcept {
  return !double_eq(a, b);
}

/// \brief Round a floating-point number to an integer.
template <typename Integer>
Integer
round(double d) noexcept { return static_cast<Integer>(d + 0.5); }

///@}

/// \defgroup vector Vectors
/// \ingroup math

/// \brief Unit-length vector.
/// \ingroup vector
///
/// Conversion from a vector to \ref unit will automatically divide the vector
/// by its magnitude; conversion from unit to unit is a no-op.
///
/// This is to help document the interface of various parts of the code and
/// to prevent bugs that stem from using non-unit length vectors where a
/// unit-length one is expected.
template <typename MatrixT>
class unit : public MatrixT {
  static_assert(MatrixT::IsVectorAtCompileTime,
                "Only vectors can be unit-length.");

  struct enabler { };

public:
  template <typename OtherMatrixT>
  unit(unit<OtherMatrixT> const& other)
    : MatrixT{other} { }

  unit(unit const&) = default;

  /// \brief Forwarding constructor.
  template <typename... Elems>
  unit(Elems... elems);

  /// \name Overrides
  ///@{
  /// These override (in a sense of static polymorphism) functions from MatrixT.

  decltype(std::declval<MatrixT>().norm())
  norm() { return 1.0; }

  decltype(std::declval<MatrixT>().squaredNorm())
  squaredNorm() { return 1.0; }

  ///@}

private:
  /// \brief Return v normalized.
  /// \throw std::invalid_argument When v is a zero vector.
  MatrixT
  normalized(MatrixT const& v) {
    double const norm_2{v.squaredNorm()};
    if (double_neq(norm_2, 0.0))
      return v / std::sqrt(norm_2);
    else
      throw std::invalid_argument{"unit: Given a zero vector"};
  }
};

/// \ingroup vector
///@{

using vector3 = Eigen::Vector3d;
using unit3   = unit<vector3>;

/// \brief Get the cosine of the directed angle from v to u.
template <
  typename Base1, typename Base2,
  typename = typename std::enable_if<
    Eigen::MatrixBase<Base1>::IsVectorAtCompileTime &&
    Eigen::MatrixBase<Base2>::IsVectorAtCompileTime &&
    static_cast<std::size_t>(Eigen::MatrixBase<Base1>::SizeAtCompileTime) ==
      static_cast<std::size_t>(Eigen::MatrixBase<Base2>::SizeAtCompileTime)
  >::type
>
auto
cos_angle(
  Eigen::MatrixBase<Base1> const& v,
  Eigen::MatrixBase<Base2> const& u
) -> decltype(u.dot(v)) 
{ return (u.dot(v)) / (u.norm() * v.norm()); }

/// \brief Get any vector perpendicular to the given one.
unit3
get_any_orthogonal(unit3 const& v);

/// \brief Get the vector reflected off a surface, given the normal vector of
/// the surface.
unit3
reflect(unit3 const& v, unit3 const& normal);

///@}

/// \defgroup rays Rays
/// \ingroup math

/// \ingroup rays
/// A ray is defined by its origin and direction; it is immutable.
///
/// Direction isn't required to be a unit vector in order to allow for
/// transformations of rays: A point on ray -- as given by \ref point_at --
/// depends on the length of the direction vector.
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

/// \ingroup rays
///@{

/// \brief Streaming support.
std::ostream&
operator << (std::ostream& out, ray const& ray);

/// \brief Transform a ray by an affine matrix.
oxatrace::ray
transform(ray const& ray, Eigen::Affine3d const& tr);

/// \brief Get a point on ray.
///
/// Given a parametric ray \f$r \;:\; [0, \infty) \to \mathbb{R}^3\f$, compute
/// \f$r(t)\f$.
///
/// \throws std::logic_error t is negative.
vector3
point_at(ray const& r, double t);

///@}

/// \brief Lazily evaluated point on ray.
/// \ingroup rays
///
/// This holds a \ref ray and a parameter for \ref point_at. It will lazily
/// compute a point on the ray, and cache the result to avoid further
/// re-evaluation.
class ray_point {
public:
  /// \throws std::logic_error t is negative.
  ray_point(oxatrace::ray const& ray, double param);

  oxatrace::ray
  ray() const noexcept      { return ray_; }

  double
  param() const noexcept    { return param_; }

  /// \brief Compute the point on ray or fetch the cached one.
  vector3
  point() const;

private:
  oxatrace::ray ray_;
  double        param_;
  mutable boost::optional<vector3> point_;
};

// Unit implementation...

template <typename MatrixT>
template <typename... Elems>
unit<MatrixT>::unit(Elems... elems)
  : MatrixT{normalized({std::forward<Elems>(elems)...})} {}

}  // namespace oxatrace

#endif
