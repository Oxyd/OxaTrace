#ifndef OXATRACE_SHAPES_HPP
#define OXATRACE_SHAPES_HPP

#include "color.hpp"
#include "math.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace oxatrace {

class light;

//
// Shapes
//

// Elementary shape is simply a shape in its basic orientation. For example,
// unit sphere centered around the origin, the xy plane, or an cylinder with
// unit diameter, unit height pointing up along the y axis and centered around
// the origin.
//
// Elementary shapes are expected to contain no non-static non-const data. This
// is to allow sharing of elementary shapes among many solids as well as their
// caching within an shape factory. This restriction also ensures
// that accessing a shared shape is thread-safe.
class shape {
public:
  using intersection_list = std::vector<double>;

  virtual
  ~shape() noexcept { }

  // Intersect this elementary shape with a ray
  virtual intersection_list
  intersect(ray const& ray) const = 0;

  // Get the normal to this shape for a given intersection point. The given
  // point is assumed to lie on the surface of this elementary shape; if this
  // isn't satisfied, the behaviour is undefined. The vector returned is the
  // one pointing out of the shape.
  virtual unit3
  normal_at(ray_point const& point) const = 0;
};

// Unit sphere centered around the origin.
class sphere final : public shape {
public:
  virtual intersection_list
  intersect(ray const&) const override;

  virtual unit3
  normal_at(ray_point const&) const override;
};

// The xy plane.
class plane final : public shape {
public:
  virtual intersection_list
  intersect(ray const&) const override;

  virtual unit3
  normal_at(ray_point const&) const override;
};

//
// Materials
//

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
  color
  base_color() const;

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
  color
  illuminate(
    color const& base_color, unit<vector3> const& normal,
    light const& light, unit<vector3> const& light_dir
  ) const;

private:
  color    ambient_;
  double   diffuse_;
  double   specular_;
  unsigned specular_exponent_;
};

//
// Renderable solids
//

// Solid is a renderable entity. It can be intersected with a ray, and has 
// various attributes associated with it, such as shape, material or texture.
class solid {
public:
  // Construct a solid of a given shape.
  solid(std::shared_ptr<oxatrace::shape> const& s, material mat);

  // Observers...
  oxatrace::material const&
  material() const noexcept;

  shape::intersection_list
  intersect(ray const& ray) const;

  unit3
  normal_at(ray_point const& rp) const;

  // Modifiers...
  // Translate this solid by a vector.
  solid&
  translate(vector3 const& tr);

  // Scale this solid by a coefficient. All axes can be scaled by the same
  // amount, or different coefficient may be specified for each axis.
  // Throws:
  //   -- std::invalid_argument: coef <= 0.0 or any of x, y, z <= 0.0.
  solid&
  scale(double coef);
  solid&
  scale(double x, double y, double z);

  // Rotate this solid around an axis.
  solid&
  rotate(Eigen::AngleAxisd const& rot);

  // Apply a generic transformation to this solid. If an inverse transformation
  // is provided, it needs to be correct, otherwise undefined results will
  // occur. If it isn't provided, one will be calculated by inverting the given
  // matrix.
  solid&
  transform(Eigen::Affine3d const& tr, Eigen::Affine3d const& inverse);
  solid&
  transform(Eigen::Affine3d const& tr);

private:
  std::shared_ptr<oxatrace::shape> shape_;
  oxatrace::material               material_;
  Eigen::Affine3d                  world_to_object_;
  Eigen::Affine3d                  object_to_world_;
};

}  // namespace oxatrace

#endif
