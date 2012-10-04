#ifndef OXATRACE_TRACER_HPP
#define OXATRACE_TRACER_HPP

#include "image.hpp"

namespace oxatrace {

class scene;

// Synthesize an image from a scene.
auto synthesize(scene const& scene, std::size_t width, std::size_t height)
  -> image;

} // namespace oxatrace

#endif

