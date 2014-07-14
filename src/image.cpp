#include "image.hpp"

#include "math.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

using namespace oxatrace;

static hdr_color::channel
clip_channel(hdr_color::channel in) {
  return in <= 1.0 ? in : 1.0;
}

double
oxatrace::log_avg_luminance(hdr_image const& image) {
  constexpr double DELTA = 0.001;

  double accum = 0.0;
  for (hdr_color c : image)
    accum += std::log(DELTA + luminance(c));

  return std::exp(accum / (image.width() * image.height()));
}

ldr_image
oxatrace::ldr_from_hdr(hdr_image const& hdr) {
  using ldr_pixel   = ldr_image::pixel_type;
  using hdr_pixel   = hdr_image::pixel_type;
  using ldr_channel = ldr_pixel::channel;
  using hdr_channel = hdr_pixel::channel;
  ldr_channel const out_max = std::numeric_limits<ldr_channel>::max();

  ldr_image result{hdr.width(), hdr.height()};
  std::transform(
    hdr.begin(), hdr.end(), result.begin(),
    [&] (hdr_pixel const& in) -> ldr_pixel {
      ldr_pixel out;
      for (std::size_t i = 0; i < ldr_pixel::CHANNELS; ++i)
        out[i] = round<ldr_channel>(clip_channel(in[i]) * out_max);
      return out;
    }
  );

  return result;
}

hdr_image
oxatrace::expose(hdr_image image, double exposure) {
  assert(exposure > 0.0);
  
  for (auto& pixel : image)
    for (auto& channel : pixel)
      channel = 1.0 - std::exp(channel * -exposure);
  return image;
}

hdr_image
oxatrace::apply_reinhard(hdr_image image, double key) {
  assert(key > 0.0);
  
  double const avg_luminance = log_avg_luminance(image);

  for (auto& pixel : image) {
    pixel *= key / avg_luminance;
    for (auto& channel : pixel)
      channel /= 1.0 + channel;
  }

  return image;
}

hdr_image
oxatrace::correct_gamma(hdr_image image, double gamma) {
  double const g = 1 / gamma;
  for (auto& pixel : image)
    for (auto& c : pixel)
      c = std::pow(c, g);
  return image;
}

void
oxatrace::save(ldr_image const& image, std::string const& filename) {
  std::string const BINARY_PPM_MAGIC = "P6";
  unsigned const    MAX_PIXEL_VALUE  =
    std::numeric_limits<ldr_image::pixel_type::channel>::max();

  std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
  out.exceptions(std::ios::badbit | std::ios::failbit);

  // Header:
  out << BINARY_PPM_MAGIC << '\n'
      << image.width() << ' ' << image.height() << '\n'
      << MAX_PIXEL_VALUE << '\n';

  // Data:
  for (auto const& pixel : image)
    for (auto const& channel : pixel)
      out.put(channel);
}
