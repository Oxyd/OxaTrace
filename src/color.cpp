#include "color.hpp"

using namespace oxatrace;

static constexpr double XYZ_RGB[xyz_color::CHANNELS][rgb_color::CHANNELS] = {
  {0.5767309, 0.1855540, 0.1881852},
  {0.2973769, 0.6273491, 0.0752741},
  {0.0270343, 0.0706872, 0.9911085}
};

static constexpr double RGB_XYZ[rgb_color::CHANNELS][xyz_color::CHANNELS] = {
  {2.0413690,  -0.5649464, -0.3446944},
  {-0.9692660,  1.8760108,  0.0415560},
  { 0.0134474, -0.1183897,  1.0154096}
};

xyz_color
oxatrace::xyz_from_rgb(rgb_color const& rgb) {
  return {
    XYZ_RGB[0][0] * rgb[0] + XYZ_RGB[0][1] * rgb[1] + XYZ_RGB[0][2] * rgb[2],
    XYZ_RGB[1][0] * rgb[0] + XYZ_RGB[1][1] * rgb[1] + XYZ_RGB[1][2] * rgb[2],
    XYZ_RGB[2][0] * rgb[0] + XYZ_RGB[2][1] * rgb[1] + XYZ_RGB[2][2] * rgb[2]
  };
}

rgb_color
oxatrace::rgb_from_xyz(xyz_color const& xyz) {
  return {
    RGB_XYZ[0][0] * xyz[0] + RGB_XYZ[0][1] * xyz[1] + RGB_XYZ[0][2] * xyz[2],
    RGB_XYZ[1][0] * xyz[0] + RGB_XYZ[1][1] * xyz[1] + RGB_XYZ[1][2] * xyz[2],
    RGB_XYZ[2][0] * xyz[0] + RGB_XYZ[2][1] * xyz[1] + RGB_XYZ[2][2] * xyz[2]
  };
}

double
oxatrace::luminance(hdr_color const& color) {
  return 0.2126 * color[0] + 0.7152 * color[1] + 0.0722 * color[2];
}
