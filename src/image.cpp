#include "image.hpp"

#include <boost/gil/image.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/typedefs.hpp>

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include <boost/gil/extension/io/png_io.hpp>

#include <algorithm>
#include <functional>

namespace oxatrace {

// Clamp a value into an interval.
static
auto clamp(double x, double lower, double upper) -> double {
  return std::max(std::min(x, upper), lower);
}

channel::channel(double value)
  : value_{clamp(value, MIN, MAX)} { }

auto channel::operator = (double d) -> channel& {
  set(d);
  return *this;
}

void channel::set(double value)     { value_ = clamp(value, MIN, MAX);}
channel::operator double () const   { return get(); }
auto channel::get() const -> double { return value_; }

auto channel::operator += (channel other) -> channel& {
  set(value_ + other.value_);
  return *this;
}

auto channel::operator -= (channel other) -> channel& {
  set(value_ - other.value_);
  return *this;
}

auto channel::operator *= (channel other) -> channel& { 
  set(value_ * other.value_);
  return *this;
}

auto channel::operator /= (channel other) -> channel& {
  set(value_ / other.value_);
  return *this;
}

color::color() { }
color::color(channel r, channel g, channel b)
  : channels_{{r, g, b}} { }

auto color::operator += (color other) -> color& {
  r() += other.r();
  g() += other.g();
  b() += other.b();
  return *this;
}

auto color::operator -= (color other) -> color& {
  r() -= other.r();
  g() -= other.g();
  b() -= other.b();
  return *this;
}

auto color::operator *= (color other) -> color& {
  r() *= other.r();
  g() *= other.g();
  b() *= other.b();
  return *this;
}

auto color::operator /= (color other) -> color& {
  r() /= other.r();
  g() /= other.g();
  b() /= other.b();
  return *this;
}

image::image(std::size_t w, std::size_t h)
  : pixels_{w * h}
  , width_{w} { }

auto image::pixel_at(std::size_t x, std::size_t y) -> color& {
  return const_cast<color&>(const_cast<image const*>(this)->pixel_at(x, y));
}

auto image::pixel_at(std::size_t x, std::size_t y) const -> color const& {
  if (x < width_ && x * y < pixels_.size())
    return pixels_[y * width_ + x];
  else
    throw std::logic_error{"image::pixel_at: Invalid coordinates"};
}

void save(image const& image, std::string const& filename) {
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

} // namespace oxatrace
