#ifndef OXATRACE_IMAGE_HPP
#define OXATRACE_IMAGE_HPP

#include "color.hpp"

#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace oxatrace {

// Stores pixels and provides interface for their direct manipulation.
//
// This is essentially a fixed-size random-access container.
template <typename PixelT>
class basic_image {
  using pixel_list = std::vector<PixelT>;

public:
  using pixel_type            = PixelT;
  using pixel_iterator        = typename pixel_list::iterator;
  using const_pixel_iterator  = typename pixel_list::const_iterator;
  using index                 = std::size_t;

  // Create an image of given dimensions.
  // Throws std::logic_error when dimensions are invalid.
  basic_image(index width, index height)
    : pixels_(width * height)
    , width_{width}
  {
    if (width == 0 || height == 0)
      throw std::logic_error{"basic_image: Cannot construct a zero-size image"};
  }

  index width() const noexcept  { return width_; }
  index height() const noexcept { return pixels_.size() / width(); }
  index size() const noexcept   { return pixels_.size(); }

  pixel_iterator
  begin() noexcept          { return pixels_.begin(); }
  pixel_iterator
  end() noexcept            { return pixels_.end(); }

  const_pixel_iterator
  begin() const noexcept    { return pixels_.begin(); }
  const_pixel_iterator
  end() const noexcept      { return pixels_.end(); }

  const_pixel_iterator
  cbegin() const noexcept   { return pixels_.begin(); }
  const_pixel_iterator
  cend() const noexcept     { return pixels_.end(); }

  // Get pixel at given coordinates.
  // Throws std::logic_error when coordinates are out of bounds
  pixel_type&
  pixel_at(index x, index y);
  pixel_type const&
  pixel_at(index x, index y) const;

private:
  pixel_list  pixels_;
  std::size_t width_;
};

// Channels in [0, +oo).
using hdr_image = basic_image<hdr_color>;

// Channels in {0, ..., 255}.
using ldr_image = basic_image<ldr_color>;

// Get the log-average luminance of a picture.
//
// Log-average luminance of a picture is defined as
//
//            ( 1                             )
// L_avg = exp(--- * sum log (delta + L(x, y))),
//            ( N    x,y                      )
//
// where
// - N is the total number of pixels
// - delta is a small positive number to avoid trouble with cases where
//   L(x, y) = 0
// - L(x, y) is the luminance of the pixel at (x,y).
//
// L_avg is the geometric mean of luminances.
double
log_avg_luminance(hdr_image const& image);

// HDR → LDR transform.
//
// Input pixels in range [0, 1] are linearly mapped and rounded to byte
// pixel values {0, ..., 255}. Input pixels > 1 are mapped to output
// 255, producing a burn-out.
ldr_image
ldr_from_hdr(hdr_image const& hdr);

// Simulate real-life film exposure.
//
// This transform image via
//
//   I_out = 1 - exp(-exposure * I_in),
//
// where exposure is a positive parameter roughly corresponding to the
// exposition time.
//
// Throws std::logic_error when exposure is non-positive.
hdr_image
expose(hdr_image image, double exposure);

// Apply the Reinhard's operator.
//
// Each channel is transformed by
//
//                   key
//      I' = I_in * -----
//                  L_avg
//
//             I'
//   I_out = ------,
//           1 + I'
//
// where key is a positive parameter and L_avg is log-average
// luminance as returned by log_avg_luminance.
//
// Throws std::logic_error when key is non-positive.
hdr_image
apply_reinhard(hdr_image image, double key = 0.18);

// Perform gamma correction.
hdr_image
correct_gamma(hdr_image image, double gamma = 2.2);

// Save an LDR image into a PPM file.
// Throws std::ios_base::failure on I/O error.
void
save(ldr_image const& image, std::string const& filename);

//
// basic_image implementation
//

template <typename PixelT>
auto
basic_image<PixelT>::pixel_at(index x, index y) -> pixel_type& {
  return const_cast<pixel_type&>(
    const_cast<basic_image const*>(this)->pixel_at(x, y)
  );
}

template <typename PixelT>
auto
basic_image<PixelT>::pixel_at(index x, index y) const -> pixel_type const& {
  if (x < width_ && x * y < pixels_.size())
      return pixels_[y * width_ + x];
  else
    throw std::logic_error{"image::pixel_at: Invalid coordinates"};
}

} // namespace oxatrace

#endif
