#include "color.hpp"

using namespace oxatrace;

double
oxatrace::luminance(hdr_color const& color) {
  return 0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2];
}
