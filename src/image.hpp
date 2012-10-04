#ifndef OXATRACE_IMAGE_HPP
#define OXATRACE_IMAGE_HPP

#include <boost/operators.hpp>

#include <tuple>
#include <vector>
#include <cstddef>
#include <array>

namespace oxatrace {

// Channel is a double in the range [0, 1]; its values are automatically clamped
// into this interval.
class channel
  : boost::field_operators<channel
  , boost::less_than_comparable<channel>> {
public:
  // Construction...
  channel(double value = 0.0);

  // Modifiers...
  auto operator = (double d) -> channel&;
  void set(double value);

  // Observers...
  operator double () const;
  auto get() const -> double;

  // Operators...
  auto operator += (channel other) -> channel&;
  auto operator -= (channel other) -> channel&;
  auto operator *= (channel other) -> channel&;
  auto operator /= (channel other) -> channel&;

private:
  static constexpr double MIN{0.0};
  static constexpr double MAX{1.0};
  
  double value_;
};

inline
auto operator < (channel lhs, channel rhs) -> bool {
  return lhs.get() < rhs.get();
}

// Colour is three channels in one: red, green, and blue. The meaning of the
// values of the channels is not pre-determined.
class color : boost::field_operators<color> {
  static constexpr std::size_t CHANNELS{3};
  using channel_list = std::array<channel, CHANNELS>;
  
public:
  // Types...
  using channel_iterator = channel_list::iterator;
  using const_channel_iterator = channel_list::const_iterator;
  
  // Construction...
  color();
  color(channel r, channel g, channel b);

  // Observers...
  auto r() -> channel& { return channels_[0]; }
  auto g() -> channel& { return channels_[1]; }
  auto b() -> channel& { return channels_[2]; }

  auto r() const -> channel const& { return channels_[0]; }
  auto g() const -> channel const& { return channels_[1]; }
  auto b() const -> channel const& { return channels_[2]; }
  
  // Iterator access over all three channels.
  auto begin() noexcept -> channel_iterator { return channels_.begin(); }
  auto end() noexcept   -> channel_iterator { return channels_.end(); }

  auto begin() const noexcept -> const_channel_iterator {
    return channels_.begin();
  }
  auto end() const noexcept -> const_channel_iterator {
    return channels_.end();
  }

  // Operators...
  // These work component-wise.
  auto operator += (color other) -> color&;
  auto operator -= (color other) -> color&;
  auto operator *= (color other) -> color&;
  auto operator /= (color other) -> color&;

private:
  channel_list channels_;
};

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
