#ifndef OXATRACE_PIXEL_HPP
#define OXATRACE_PIXEL_HPP

#include <boost/operators.hpp>

#include <array>
#include <cstdint>

namespace oxatrace {

struct rgb_tag { enum channels { r, g, b }; };
struct xyz_tag { enum channels { x, y, z }; };

// Baisc colour is three channels in one. The representation, and thus the
// range, of individual channels is given by the template parameter ChannelT.
// Overflows are not detected and result in undefined behaviour.
//
// Basic colour supports all field operators as well as multiplication by scalar
// of type ChannelT.
//
// The channel type must not throw exceptions.
//
// This is a generic template for any three-component colour space. Exact
// colour space is specified by the ColorSpace template parameter which make it
// possible to check at compile-time for errors stemming from mixing triples
// from different colour spaces.
template <typename ChannelT, typename ColorSpace>
class basic_color
  : boost::field_operators<basic_color<ChannelT, ColorSpace>>
  , boost::multipliable<basic_color<ChannelT, ColorSpace>, ChannelT>
  , boost::dividable<basic_color<ChannelT, ChannelT>, ChannelT>
{
public:
  static constexpr std::size_t CHANNELS{3};

private:
  using channel_list = std::array<ChannelT, CHANNELS>;
  
public:
  // Types...

  using channel                 = ChannelT;
  using channel_iterator        = typename channel_list::iterator;
  using const_channel_iterator  = typename channel_list::const_iterator;
  
  // Construction...

  basic_color() noexcept { }  // Leaves channels uninitialised
  basic_color(channel r, channel g, channel b) noexcept;

  // Observers...

  channel&
  operator [] (std::size_t i) noexcept          { return channels_[i]; }

  channel const&
  operator [] (std::size_t i) const noexcept    { return channels_[i]; }

  // Iterator access over all channels.
  channel_iterator
  begin() noexcept          { return channels_.begin(); }
  channel_iterator
  end() noexcept            { return channels_.end(); }

  const_channel_iterator
  begin() const noexcept    { return channels_.begin(); }
  const_channel_iterator
  end() const noexcept      { return channels_.end(); }

  // Operators...

  // These work component-wise.
  basic_color& operator += (basic_color other) noexcept;
  basic_color& operator -= (basic_color other) noexcept;
  basic_color& operator *= (basic_color other) noexcept;
  basic_color& operator /= (basic_color other) noexcept;

  // These multiply/divide each channel separately.
  basic_color& operator *= (ChannelT d) noexcept;
  basic_color& operator /= (ChannelT d) noexcept;

private:
  channel_list channels_;
};

using hdr_color = basic_color<double, rgb_tag>;
using ldr_color = basic_color<std::uint8_t, rgb_tag>;

using rgb_color = basic_color<double, rgb_tag>;  // Same as hdr_color
using xyz_color = basic_color<double, xyz_tag>;

xyz_color
xyz_from_rgb(rgb_color const& rgb);

rgb_color
rgb_from_xyz(xyz_color const& xyz);

double
luminance(hdr_color const& color);

//
// basic_color implementation...
//

template <typename ChannelT, typename ColorSpace>
basic_color<ChannelT, ColorSpace>::basic_color(
  ChannelT r, ChannelT g, ChannelT b
) noexcept
  : channels_{r, g, b}
{ }

#define impl_op(op)                                                            \
  template <typename ChannelT, typename ColorSpace>                            \
  basic_color<ChannelT, ColorSpace>&                                           \
  basic_color<ChannelT, ColorSpace>::operator op (basic_color other) noexcept {\
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
  template <typename ChannelT, typename ColorSpace>                            \
  basic_color<ChannelT, ColorSpace>&                                           \
  basic_color<ChannelT, ColorSpace>::operator op (ChannelT d) noexcept {       \
    for (std::size_t i = 0; i < CHANNELS; ++i)                                 \
      channels_[i] op d;                                                       \
    return *this;                                                              \
  }

impl_op2(*=)
impl_op2(/=)

#undef impl_op2

}

#endif

