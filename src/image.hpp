#ifndef OXATRACE_IMAGE_HPP
#define OXATRACE_IMAGE_HPP

#include "color.hpp"

#include <vector>
#include <cstddef>

namespace oxatrace {

// We want somehow to incorporate gamma correction and exposure here. Ray
// tracing will give us an HDR image -- floating-point values ranging from 0.0
// to an arbitrary maximum. However, we want to output an LDR image. To do that,
// we are going to need to compress the [0, inf) floating-point range into an
// [0, 256) integer range.
//
// Going from [0, inf) to [0, 256) is called tone mapping. There are various
// ways to go about tone mapping; implemented in this program is an "exposure"
// operator that simulates the exposure of the photo-sensitive film used in
// classical cameras.

// Image is a 2D array of colours. The particular meaning of the channels'
// values is still unspecified at this point.
class image {
  using pixel_list = std::vector<color>;

public:
  // Types...
  using pixel_iterator        = pixel_list::iterator;
  using const_pixel_iterator  = pixel_list::const_iterator;
  using index                 = std::size_t;

  // Construction...
  // Create an image of given dimensions.
  // Throws:
  //   -- std::logic_error: Dimensions are invalid.
  image(index width, index height);

  // Observers...
  index width() const   { return width_; }
  index height() const  { return pixels_.size() / width(); }

  // Pixel access...
  pixel_iterator
  begin()   { return pixels_.begin(); }
  pixel_iterator
  end()     { return pixels_.end(); }

  const_pixel_iterator
  begin() const {
    return pixels_.begin();
  }
  const_pixel_iterator
  end() const {
    return pixels_.end();
  }

  // Get pixel at given coordinates.
  // Throws:
  //   -- std::logic_error: Coordinates out of bounds.
  color&
  pixel_at(index x, index y);
  color const&
  pixel_at(index x, index y) const;

private:
  pixel_list  pixels_;
  std::size_t width_;
};

// Save an image into a PNG file.
// Throws:
//   -- std::ios_base::failure: I/O error.
void
save(image const& image, std::string const& filename);
 
} // namespace oxatrace

#endif
