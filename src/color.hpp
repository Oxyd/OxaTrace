#ifndef OXATRACE_COLOR_HPP
#define OXATRACE_COLOR_HPP

#include <boost/operators.hpp>

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


}

#endif

