#ifndef OXATRACE_SHAPES_HPP
#define OXATRACE_SHAPES_HPP

#include "color.hpp"
#include "math.hpp"

#include <memory>
#include <stdexcept>
#include <tuple>

namespace oxatrace {

class light;

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
  sphere(vector3 center, double radius);

  virtual auto intersect_both(ray const& r) const override 
    -> both_intersections;
  virtual auto normal_at(vector3 point) const override -> unit<vector3>;

private:
  vector3 center_;
  double  radius_;
};

// Materials -------------------------------------------------------------------

// Material defines the various visual qualities of a solid. It is responsible
// for giving rays their colour based on which light sources illuminate the
// given solid at given point.
class material {
public:
  // Create a Phong material.
  // Throws:
  //   -- std::invalid_argument: If diffuse or specular are outside the range
  //                             [0, 1].
  material(color const& ambient, double diffuse, double specular,
           unsigned specular_exponent);

  // Get the base colour of the material. This is the colour the object should
  // have even if it is unaffected by any light source. In other words, the
  // ambient colour.
  auto base_color() const -> color;

  // Given a light source directly visible from a given point, update the
  // resulting ray colour accordingly. During ray tracing, call this function
  // once for each directly visible light source for any given ray-solid 
  // intersection.
  //
  // Parameters:
  //   -- base_color: The ray colour computed so far.
  //   -- normal:     Surface normal at the point of intersection.
  //   -- light_dir:  Direction (in world coordinates) toward the source of
  //                  light from the intersection point.
  //   -- light:      The light illuminating the solid.
  auto illuminate(
    color const& base_color, unit<vector3> const& normal,
    light const& light, unit<vector3> const& light_dir
  ) const -> color;

private:
  color    ambient_;
  double   diffuse_;
  double   specular_;
  unsigned specular_exponent_;
};

// Renderable solids -----------------------------------------------------------

// Solid is a renderable entity. It can be intersected with a ray, and has 
// various attributes associated with it, such as colour, material or texture.
class solid {
public:
  // Construct a solid of a given shape.
  solid(std::shared_ptr<oxatrace::shape> const& s, material mat);

  auto shape() const noexcept    -> oxatrace::shape const&;
  auto material() const noexcept -> oxatrace::material const&;

private:
  std::shared_ptr<oxatrace::shape> shape_;
  oxatrace::material               material_;
};

}  // namespace oxatrace

#endif
