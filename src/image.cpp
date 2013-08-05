#include "image.hpp"

#include "math.hpp"

#include <cassert>

using namespace oxatrace;

// Map hdr_pixels in range [0, 1] to ldr_pixels in range {0, ..., 255}.
ldr_image::pixel_type
oxatrace::to_ldr(ldr_float_image::pixel_type pixel) {
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

ldr_float_image::pixel_type
oxatrace::clip(hdr_image::pixel_type pixel) {
  auto do_clip = [] (hdr_image::pixel_type::channel c) {
    return c < 1.0 ? c : 1.0;
  };

  return {
    do_clip(pixel.r()),
    do_clip(pixel.g()),
    do_clip(pixel.b())
  };
}

ldr_float_image::pixel_type
oxatrace::exposition::operator () (hdr_image::pixel_type pixel) const noexcept {
  return {
    1.0 - std::exp(pixel.r() * -exposure_),
    1.0 - std::exp(pixel.g() * -exposure_),
    1.0 - std::exp(pixel.b() * -exposure_)
  };
}

ldr_float_image::pixel_type
oxatrace::gamma_correction::operator () (ldr_float_image::pixel_type pixel)
const noexcept
{
  double const g = 1 / gamma_;
  return {
    std::pow(pixel.r(), g),
    std::pow(pixel.g(), g),
    std::pow(pixel.b(), g)
  };
}
