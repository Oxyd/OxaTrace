#ifndef OXATRACE_TRACER_HPP
#define OXATRACE_TRACER_HPP

#include "color.hpp"

namespace oxatrace {

class ray;
class scene;

// Trace a ray through a scene, recursively.
//
// XXX: Config:
//   -- background colour
//   -- don't pass max_depth explicitely?
//   Probably don't want a dependency on some global config type. Better have
//   some local config type. But then that will have to be passed around...
//
//   Could have a class. But then the only purpose of this class would be to
//   store some configuration and expose one public member function that would
//   do the raytracing.
//
//   Have a more potent class, then? One that can synthesise a whole image?
//   Well, but I might want the trace function for other purposes than 
//   image synthesis.
//
//   Have this thing with "a lot" of parameters, and then -- if desired --
//   create a binder that will just bind some of the parameters? Functional
//   programming-style!
//
//   We want some way to introduce perturbations into the algorithm, though...
//   Fuzzy reflections. They'll also have to have access to Phong's H...
//   That can be hidden inside material, though.
//
//   Have some policy classes that can then be passed into here?
color
trace(ray const& ray, scene const& scene, unsigned max_depth = 1);

} // namespace oxatrace

#endif

