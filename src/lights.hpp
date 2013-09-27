#ifndef OXATRACE_LIGHTS_HPP
#define OXATRACE_LIGHTS_HPP

#include "color.hpp"
#include "math.hpp"

namespace oxatrace {

// Light is some "invisible" source of light. It is defined by its shape,
// colour and intensity. Lights are usually not considered while tracing 
// primary or secondary rays, instead special rays are sent to each light source
// to determine the illumination of an object.
//
// Intensity of this light source is implicitly defined by the given colour.
class light {
public:
  // XXX: How is this useful? Every light is going to have a colour. What they
  // are going to differ in, though, is their shape. Perhaps have light contain
  // the colour and some abstract shape, and allow the shape to be sampled
  // when needed for raytracing.

  virtual ~light() { }

  // Get a point in the light.
  virtual vector3
  get_source() const = 0;

  // Get the colour of this light.
  virtual hdr_color
  color() const = 0;
};

// Point light source is a simple model where all light is emitted from a single
// infinitesimal point in space.
class point_light final : public light {
public:
  // Construct a point light from the position of its only point and its colour.
  point_light(vector3 const& pos, hdr_color const& col);

  virtual vector3
  get_source() const override;

  virtual hdr_color
  color() const override;

private:
  vector3   position_;
  hdr_color color_;
};

} // namespace oxatrace

#endif

