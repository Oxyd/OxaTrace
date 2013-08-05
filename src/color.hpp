#ifndef OXATRACE_PIXEL_HPP
#define OXATRACE_PIXEL_HPP

#include <boost/operators.hpp>

#include <array>
#include <cstdint>

namespace oxatrace {

// Baisc colour is three channels in one: red, green, and blue. The
// representation, and thus the range, of individual channels is given by the
// template parameter ChannelT. Overflows are not detected and result in
// undefined behaviour.
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
  static constexpr std::size_t CHANNELS{3};
  using channel_list = std::array<ChannelT, CHANNELS>;
  
public:
  // Types...
  using channel = ChannelT;
  using channel_iterator = typename channel_list::iterator;
  using const_channel_iterator = typename channel_list::const_iterator;
  
  // Construction...
  basic_color() noexcept { }  // Leaves channels uninitialised
  basic_color(channel r, channel g, channel b) noexcept;

  // Observers...
  channel& r() noexcept             { return channels_[0]; }
  channel& g() noexcept             { return channels_[1]; }
  channel& b() noexcept             { return channels_[2]; }

  channel const& r() const noexcept { return channels_[0]; }
  channel const& g() const noexcept { return channels_[1]; }
  channel const& b() const noexcept { return channels_[2]; }
  
  // Iterator access over all three channels.
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

using hdr_color = basic_color<double>;
using ldr_color = basic_color<std::uint8_t>;

//
// basic_color implementation...
//

template <typename ChannelT>
basic_color<ChannelT>::basic_color(ChannelT r, ChannelT g, ChannelT b) noexcept
  : channels_{r, g, b}
{ }

#define impl_op(op)                                                            \
  template <typename ChannelT>                                                 \
  basic_color<ChannelT>&                                                       \
  basic_color<ChannelT>::operator op (basic_color other) noexcept              \
  { r() op other.r(); g() op other.g(); b() op other.b(); return *this; }

impl_op(+=)
impl_op(-=)
impl_op(*=)
impl_op(/=)

#undef impl_op
#define impl_op2(op)                                                           \
  template <typename ChannelT>                                                 \
  basic_color<ChannelT>&                                                       \
  basic_color<ChannelT>::operator op (ChannelT d) noexcept                     \
  { r() op d; g() op d; b() op d; return *this; }

impl_op2(*=)
impl_op2(/=)

#undef impl_op2

}

#endif

