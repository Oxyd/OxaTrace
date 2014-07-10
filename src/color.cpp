#include "color.hpp"

#include <cmath>

using namespace oxatrace;

/// See http://en.wikipedia.org/wiki/Luminance
/// See http://en.wikipedia.org/wiki/Luma_(video)
double
oxatrace::luminance(hdr_color const& color) {
  return 0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2];
}

double
oxatrace::distance(hdr_color x, hdr_color y) {
  double sum{};
  for (std::size_t channel = 0; channel < hdr_color::CHANNELS; ++channel) {
    double const d = x[channel] - y[channel];
    sum += d * d;
  }

  return std::sqrt(sum);
}
