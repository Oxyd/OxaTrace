#include "color.hpp"

using namespace oxatrace;

/// \see http://en.wikipedia.org/wiki/Luminance
/// \see http://en.wikipedia.org/wiki/Luma_(video)
double
oxatrace::luminance(hdr_color const& color) {
  return 0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2];
}
