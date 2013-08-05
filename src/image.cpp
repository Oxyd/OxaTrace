#include "image.hpp"

#include "math.hpp"

oxatrace::ldr_image::pixel_type
oxatrace::clip(hdr_image::pixel_type pixel) {
  auto clip = [] (hdr_image::pixel_type::channel c) {
    return c < 1.0 ? c : 1.0;
  };

  using ldr_channel = ldr_image::pixel_type::channel;
  return ldr_image::pixel_type{
    round<ldr_channel>(255.0 * clip(pixel.r())),
    round<ldr_channel>(255.0 * clip(pixel.g())),
    round<ldr_channel>(255.0 * clip(pixel.b()))
  };
}
