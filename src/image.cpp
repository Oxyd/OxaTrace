#include "image.hpp"

#include <boost/gil/image.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/typedefs.hpp>

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include <boost/gil/extension/io/png_io.hpp>

#include <algorithm>

using namespace oxatrace;

image::image(std::size_t w, std::size_t h)
  : pixels_{w * h}
  , width_{w} { }

color&
image::pixel_at(std::size_t x, std::size_t y) {
  return const_cast<color&>(const_cast<image const*>(this)->pixel_at(x, y));
}

color const&
image::pixel_at(std::size_t x, std::size_t y) const {
  if (x < width_ && x * y < pixels_.size())
    return pixels_[y * width_ + x];
  else
    throw std::logic_error{"image::pixel_at: Invalid coordinates"};
}

void
oxatrace::save(image const& image, std::string const& filename) {
  // We're going to copy the image over into a new one that Boost.GIL can 
  // understand. We're then going to save the copy. It appears to be easier
  // this way than trying to force Boost.GIL to understand oxatrace::image...
  
  using channel_t = unsigned char;
  using pixel_t   = boost::gil::rgb8_pixel_t;
  using image_t   = boost::gil::rgb8_image_t;

  auto max = boost::gil::channel_traits<channel_t>::max_value();
  auto convert = [&](channel const& chan) {
    return static_cast<channel_t>(max * chan.get() + 0.5);
  };

  image_t im(image.width(), image.height());
  auto view = boost::gil::view(im);

  std::transform(image.begin(), image.end(), view.begin(),
                 [&](color const& c) {
                   return pixel_t{convert(c.r()),
                                  convert(c.g()),
                                  convert(c.b())};
                 });
  boost::gil::png_write_view(filename, view);
}

