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
  for (auto& pixel : image)
    for (auto& c : pixel)
      c = 1.0 - std::exp(c * -exposure);
  return image;
}

hdr_image
oxatrace::apply_reinhard(hdr_image image, double key) {
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
  std::ofstream out(filename.c_str());
  out.exceptions(std::ios::badbit | std::ios::failbit);

  // Header:
  out << "P6\n"     // Binary PPM
      << image.width() << ' ' << image.height() << '\n'
      << "255\n";   // Max value of a single pixel.

  // Data:
  for (auto const& pixel : image)
    for (auto const& channel : pixel)
      out.put(channel);
}
