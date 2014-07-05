#ifndef OXATRACE_PIXEL_HPP
#define OXATRACE_PIXEL_HPP

#include <boost/operators.hpp>

#include <array>
#include <cstdint>

namespace oxatrace {

// Colour value of a single pixel.
//
// Basic colour is three channels in one. The representation, and thus the
// range, of individual channels is given by the template parameter ChannelT.
// Overflows are not detected and result in undefined behaviour.
//
// Basic colour supports all field operators as well as multiplication by scalar
// of type ChannelT.
//
// The channel type must not throw exceptions.
template <typename ChannelT>
class basic_color
  : boost::field_operators<basic_color<ChannelT>>
  , boost::multipliable<basic_color<ChannelT>, ChannelT>
  , boost::dividable<basic_color<ChannelT>, ChannelT>
{
public:
  static constexpr std::size_t CHANNELS{3};  // Number of channels in a pixel.

private:
  using channel_list = std::array<ChannelT, CHANNELS>;
  
public:
  using channel                 = ChannelT;
  using channel_iterator        = typename channel_list::iterator;
  using const_channel_iterator  = typename channel_list::const_iterator;

  basic_color() noexcept { }  // Leaves channels uninitialised
  basic_color(channel r, channel g, channel b) noexcept;

  channel&
  operator [] (std::size_t i) noexcept          { return channels_[i]; }

  channel const&
  operator [] (std::size_t i) const noexcept    { return channels_[i]; }

  channel_iterator
  begin() noexcept          { return channels_.begin(); }
  channel_iterator
  end() noexcept            { return channels_.end(); }

  const_channel_iterator
  begin() const noexcept    { return channels_.begin(); }
  const_channel_iterator
  end() const noexcept      { return channels_.end(); }

  const_channel_iterator
  cbegin() const noexcept   { return channels_.begin(); }
  const_channel_iterator
  cend() const noexcept     { return channels_.end(); }

  basic_color& operator += (basic_color other) noexcept;
  basic_color& operator -= (basic_color other) noexcept;
  basic_color& operator *= (basic_color other) noexcept;
  basic_color& operator /= (basic_color other) noexcept;

  basic_color& operator *= (ChannelT d) noexcept;
  basic_color& operator /= (ChannelT d) noexcept;

private:
  channel_list channels_;
};

using hdr_color = basic_color<double>;
using ldr_color = basic_color<std::uint8_t>;

// Get the luminance of a pixel.
double
luminance(hdr_color const& color);

//
// basic_color implementation...
//

template <typename ChannelT>
basic_color<ChannelT>::basic_color(
  ChannelT r, ChannelT g, ChannelT b
) noexcept
  : channels_{r, g, b}
{ }

#define impl_op(op)                                                            \
  template <typename ChannelT>                                                 \
  basic_color<ChannelT>&                                                       \
  basic_color<ChannelT>::operator op (basic_color other) noexcept {            \
    for (std::size_t i = 0; i < CHANNELS; ++i)                                 \
      channels_[i] op other.channels_[i];                                      \
    return *this;                                                              \
  }

impl_op(+=)
impl_op(-=)
impl_op(*=)
impl_op(/=)

#undef impl_op
#define impl_op2(op)                                                           \
  template <typename ChannelT>                                                 \
  basic_color<ChannelT>&                                                       \
  basic_color<ChannelT>::operator op (ChannelT d) noexcept {                   \
    for (std::size_t i = 0; i < CHANNELS; ++i)                                 \
      channels_[i] op d;                                                       \
    return *this;                                                              \
  }

impl_op2(*=)
impl_op2(/=)

#undef impl_op2

}

#endif

