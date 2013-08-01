#ifndef OXATRACE_COLOR_HPP
#define OXATRACE_COLOR_HPP

#include <boost/operators.hpp>

#include <array>

namespace oxatrace {

// Channel is a double in the range [0, 1]; its values are automatically clamped
// into this interval.
class channel
  : boost::field_operators<channel>
  , boost::less_than_comparable<channel> {
public:
  // Construction...
  channel(double value = 0.0);

  // Modifiers...
  channel&  operator = (double d);
  void      set(double value);

  // Observers...
  operator  double () const;
  double    get() const;

  // Operators...
  channel& operator += (channel other);
  channel& operator -= (channel other);
  channel& operator *= (channel other);
  channel& operator /= (channel other);

private:
  static constexpr double MIN{0.0};
  static constexpr double MAX{1.0};
  
  double value_;
};

inline bool
operator < (channel lhs, channel rhs) {
  return lhs.get() < rhs.get();
}

// Colour is three channels in one: red, green, and blue. The meaning of the
// values of the channels is not pre-determined.
class color 
  : boost::field_operators<color>
  , boost::multipliable<color, double>
  , boost::dividable<color, double> {
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
  channel& r() { return channels_[0]; }
  channel& g() { return channels_[1]; }
  channel& b() { return channels_[2]; }

  channel const& r() const { return channels_[0]; }
  channel const& g() const { return channels_[1]; }
  channel const& b() const { return channels_[2]; }
  
  // Iterator access over all three channels.
  channel_iterator
  begin() noexcept  { return channels_.begin(); }
  channel_iterator
  end() noexcept    { return channels_.end(); }

  const_channel_iterator
  begin() const noexcept    { return channels_.begin(); }
  const_channel_iterator
  end() const noexcept      { return channels_.end(); }

  // Operators...
  // These work component-wise.
  color& operator += (color other);
  color& operator -= (color other);
  color& operator *= (color other);
  color& operator /= (color other);

  // These multiply/divide each channel separately.
  color& operator *= (double d);
  color& operator /= (double d);

private:
  channel_list channels_;
};


}

#endif

