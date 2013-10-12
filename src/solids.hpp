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

/// \defgroup shapes Shapes
/// \ingroup scene

/// \brief A shape in its basic orientation.
///
/// Elementary shape is simply a shape in its basic orientation. For example,
/// unit sphere centered around the origin, the xy plane, or a cylinder with
/// unit diameter, unit height pointing up along the y axis and centered around
/// the origin.
///
/// Elementary shapes are expected to contain no non-static non-const data. This
/// is to allow sharing of elementary shapes among many solids as well as their
/// caching within an shape factory. This restriction also ensures
/// that accessing a shared shape is thread-safe.
class shape {
public:
  using intersection_list = std::vector<double>;

  virtual
  ~shape() noexcept { }

  /// \brief Intersect this elementary shape with a ray
  virtual intersection_list
  intersect(ray const& ray) const = 0;

  /// \brief Get the normal to this shape for a given intersection point.
  ///
  ///
  /// The given point is assumed to lie on the surface of this elementary 
  /// shape; if this isn't satisfied, the behaviour is undefined. The vector 
  /// returned is the one pointing out of the shape.
  virtual unit3
  normal_at(ray_point const& point) const = 0;
};

/// \brief Unit sphere centered around the origin.
/// \ingroup shapes
class sphere final : public shape {
public:
  virtual intersection_list
  intersect(ray const&) const override;

  virtual unit3
  normal_at(ray_point const&) const override;
};

/// \brief The xy plane.
/// \ingroup shapes
class plane final : public shape {
public:
  virtual intersection_list
  intersect(ray const&) const override;

  virtual unit3
  normal_at(ray_point const&) const override;
};

/// \defgroup materials Materials
/// \ingroup scene

/// \ingroup materials
/// Material defines the various visual qualities of a solid. It is responsible
/// for giving rays their colour based on which light sources illuminate the
/// given solid at given point.
///
/// Modelled here is a Phong material.
class material {
public:
  /// \brief Create a Phong material.
  /// \throws std::invalid_argument: diffuse, specular, or reflectance are 
  ///                                outside the range [0, 1].
  material(hdr_color const& ambient, double diffuse, double specular,
           unsigned specular_exponent,
           double reflectance = 0.0);

  /// \brief Get the base colour of the material. 
  ///
  /// This is the colour the object should have even if it is unaffected by any 
  /// light source. In other words, the ambient colour.
  hdr_color
  base_color() const { return ambient_; }

  /// \brief The reflectance of this material. 
  ///
  /// This is a value in range \f$[0, 1]\f$.
  double
  reflectance() const { return reflectance_; }

  /// \brief Update ray's colour.
  ///
  /// Given a light source directly visible from a given point, update the
  /// resulting ray colour accordingly. During ray tracing, call this function
  /// once for each directly visible light source for any given ray-solid 
  /// intersection.
  ///
  /// \param base_color  The ray colour computed so far.
  /// \param normal      Surface normal at the point of intersection.
  /// \param light_dir   Direction (in world coordinates) toward the source of
  ///                    light from the intersection point.
  /// \param light_color Colour of the light illuminating the solid.
  hdr_color
  blend_light(
    hdr_color const& base_color, unit3 const& normal,
    hdr_color const& light_color, unit3 const& light_dir
  ) const;

  /// \brief Update ray's colour after a reflection.
  ///
  /// Given a computed colour value of a reflected ray, update the resulting ray
  /// colour.
  hdr_color
  blend_reflection(hdr_color const& base_color,
                   hdr_color const& reflection_color) const;

private:
  hdr_color ambient_;
  double    diffuse_;
  double    specular_;
  unsigned  specular_exponent_;
  double    reflectance_;
};

/// \defgroup solids Renderable solids
/// \ingroup scene

/// \brief Renderable entity.
/// \ingroup solids
///
/// Solid is a renderable entity. It can be intersected with a ray, and has 
/// various attributes associated with it, such as shape, material or texture.
class solid {
public:
  /// \brief Construct a solid of a given shape.
  solid(std::shared_ptr<oxatrace::shape> const& s, material mat);

  oxatrace::material const&
  material() const noexcept;

  shape::intersection_list
  intersect(ray const& ray) const;

  unit3
  normal_at(ray_point const& rp) const;

  /// \brief Translate this solid by a vector.
  solid&
  translate(vector3 const& tr);

  /// \brief Scale this solid by a coefficient.
  ///
  /// Scale this solid by a coefficient. All axes can be scaled by the same
  /// amount, or different coefficient may be specified for each axis.
  /// 
  ///
  /// \throws std::invalid_argument: coef <= 0.0 or any of x, y, z <= 0.0.
  ///@{
  solid&
  scale(double coef);
  solid&
  scale(double x, double y, double z);
  ///@}

  /// \brief Rotate this solid around an axis.
  solid&
  rotate(Eigen::AngleAxisd const& rot);

  /// \brief Transform this solid.
  ///
  /// Apply a generic transformation to this solid. If an inverse transformation
  /// is provided, it needs to be correct, otherwise undefined results will
  /// occur. If it isn't provided, one will be calculated by inverting the given
  /// matrix.
  ///@{
  solid&
  transform(Eigen::Affine3d const& tr, Eigen::Affine3d const& inverse);
  solid&
  transform(Eigen::Affine3d const& tr);
  ///@}

private:
  std::shared_ptr<oxatrace::shape> shape_;
  oxatrace::material               material_;
  Eigen::Affine3d                  world_to_object_;
  Eigen::Affine3d                  object_to_world_;
};

}  // namespace oxatrace

#endif
