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
struct light {
  virtual ~light() { }

  // Get a point in the light.
  virtual auto get_source() const -> vector3 = 0;

  // Get the colour of this light.
  virtual auto color() const -> oxatrace::color = 0;
};

// Point light source is a simple model where all light is emitted from a single
// infinitesimal point in space.
class point_light final : public light {
public:
  // Construct a point light from the position of its only point and its colour.
  point_light(vector3 const& pos, oxatrace::color const& col);

  virtual auto get_source() const override -> vector3;
  virtual auto color() const override -> oxatrace::color;

private:
  vector3         position_;
  oxatrace::color color_;
};

} // namespace oxatrace

#endif
