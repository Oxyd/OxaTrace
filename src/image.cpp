#include "image.hpp"

#include "math.hpp"

#include <cassert>

using namespace oxatrace;

// Map hdr_pixels in range [0, 1] to ldr_pixels in range {0, ..., 255}.
static ldr_image::pixel_type
ldr_from_hdr(hdr_image::pixel_type pixel) {
  assert(pixel.r() >= 0.0 && pixel.r() <= 1.0);
  assert(pixel.g() >= 0.0 && pixel.g() <= 1.0);
  assert(pixel.b() >= 0.0 && pixel.b() <= 1.0);

  using ldr_channel = ldr_image::pixel_type::channel;
  auto rnd = round<ldr_channel>;
  return {
    rnd(255.0 * pixel.r()),
    rnd(255.0 * pixel.g()),
    rnd(255.0 * pixel.b())
  };
}

ldr_image::pixel_type
oxatrace::clip(hdr_image::pixel_type pixel) {
  auto clip = [] (hdr_image::pixel_type::channel c) {
    return c < 1.0 ? c : 1.0;
  };

  return ldr_from_hdr({
    clip(pixel.r()),
    clip(pixel.g()),
    clip(pixel.b())
  });
}

ldr_image::pixel_type
oxatrace::exposition::operator () (hdr_image::pixel_type pixel) const noexcept {
  return ldr_from_hdr({
    1.0 - std::exp(pixel.r() * -exposure_),
    1.0 - std::exp(pixel.g() * -exposure_),
    1.0 - std::exp(pixel.b() * -exposure_)
  });
}
