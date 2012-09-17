#ifndef OXATRACE_SHAPES_HPP
#define OXATRACE_SHAPES_HPP

#include "math.hpp"

#include <stdexcept>
#include <tuple>
#include <memory>

namespace oxatrace {

// Shapes ----------------------------------------------------------------------

// Shape defines the set of points occupied by a solid, or a part thereof 
// (e.g. when used in conjunction with CSG). This set is defined implicitly,
// using the intersect() member function.
struct shape {
  using both_intersections = std::tuple<vector3, vector3>;

  virtual ~shape() noexcept { }

  // Given a ray, get the intersection point closest to the ray origin.
  // Returns:
  //   -- nonzero vector: Intersection coordinates,
  //   -- zero vector:    No intersection with the ray.
  virtual auto intersect(ray const& r) const -> vector3;

  // Given a ray, get the two intersection points closest to the ray origin.
  // Returns:
  //   -- (nonzero, nonzero): Two intersection points,
  //   -- (nonzero, zero):    Ray is tangent to the shape.
  //   -- (zero, zero):       No intersection.
  virtual auto intersect_both(ray const& r) const -> both_intersections = 0;

  // Given a point on the shape, return the normal vector at that point.
  // Throws:
  //   -- std::logic_error: Point is not on the surface of the shape.
  virtual auto normal_at(vector3 point) const -> unit<vector3> = 0;
};

// A sphere is defined by its centre point and radius.
class sphere final : public shape {
public:
  // Throws std::logic_error if radius <= 0.0.
  sphere(vector3 center, double radius)
    : center_{center}
    , radius_{radius} { 
    if (radius <= 0.0) throw std::logic_error("sphere: radius <= 0.0");
  }

  virtual auto intersect_both(ray const& r) const override -> both_intersections;
  virtual auto normal_at(vector3 point) const override -> unit<vector3>;

private:
  vector3 center_;
  double  radius_;
};

// Renderable solids -----------------------------------------------------------

// Solid is a renderable entity. It can be intersected with a ray, and has 
// various attributes associated with it, such as colour, material or texture.
class solid {
public:
  // Construct a solid of a given shape.
  explicit solid(std::shared_ptr<oxatrace::shape> const& s);

  // Access to this solid's shape.
  auto shape() const noexcept -> oxatrace::shape const&;

private:
  std::shared_ptr<oxatrace::shape> shape_;
};

}

#endif
