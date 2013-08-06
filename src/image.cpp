#include "image.hpp"

#include "math.hpp"

#include <cassert>

using namespace oxatrace;

double
oxatrace::log_avg_luminance(hdr_image const& image) {
  constexpr double DELTA = 0.001;

  double accum = 0.0;
  for (hdr_color c : image)
    accum += std::log(DELTA + luminance(c));

  return std::exp(accum / (image.width() * image.height()));
}

ldr_image::pixel_type
oxatrace::to_ldr(ldr_float_image::pixel_type pixel) {
#ifndef NDEBUG
  for (auto channel : pixel)
    assert(channel >= 0 && channel <= 1.0);
#endif

  using ldr_channel = ldr_image::pixel_type::channel;
  auto rnd = round<ldr_channel>;
  return {
    rnd(255.0 * pixel[0]),
    rnd(255.0 * pixel[1]),
    rnd(255.0 * pixel[2])
  };
}

ldr_float_image::pixel_type
oxatrace::clip(hdr_image::pixel_type pixel) {
  for (auto& c : pixel)
    c = c < 1.0 ? c : 1.0;
  return pixel;
}

ldr_float_image::pixel_type
oxatrace::exposition::operator () (hdr_image::pixel_type pixel) const noexcept {
  for (auto& c : pixel)
    c = 1.0 - std::exp(c * -exposure_);
  return pixel;
}

ldr_float_image::pixel_type
oxatrace::reinhard::operator () (hdr_image::pixel_type pixel) const noexcept {
  pixel *= (key_ / avg_);
  for (auto& channel : pixel)
    channel = channel / (1.0 + channel);
  return pixel;
}

ldr_float_image::pixel_type
oxatrace::gamma_correction::operator () (ldr_float_image::pixel_type pixel)
const noexcept
{
  double const g = 1 / gamma_;
  for (auto& c : pixel)
    c = std::pow(c, g);
  return pixel;
}

