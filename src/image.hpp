#ifndef OXATRACE_IMAGE_HPP
#define OXATRACE_IMAGE_HPP

#include "color.hpp"

#include <vector>
#include <cstddef>

namespace oxatrace {

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
  auto width() const  -> index { return width_; }
  auto height() const -> index { return pixels_.size() / width(); }

  // Pixel access...
  auto begin() -> pixel_iterator { return pixels_.begin(); }
  auto end()   -> pixel_iterator { return pixels_.end(); }
  auto begin() const -> const_pixel_iterator {
    return pixels_.begin();
  }
  auto end() const -> const_pixel_iterator {
    return pixels_.end();
  }

  // Get pixel at given coordinates.
  // Throws:
  //   -- std::logic_error: Coordinates out of bounds.
  auto pixel_at(index x, index y)       -> color&;
  auto pixel_at(index x, index y) const -> color const&;

private:
  pixel_list  pixels_;
  std::size_t width_;
};

// Save an image into a PNG file.
// Throws:
//   -- std::ios_base::failure: I/O error.
void save(image const& image, std::string const& filename);
 
} // namespace oxatrace

#endif
