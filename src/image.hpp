/// \file image.hpp

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

/// \defgroup image Image handling

/// \brief Stores pixels and provides interface for their direct manipulation.
///
/// This is essentially a fixed-size random-access container.
///
/// \ingroup image
template <typename PixelT>
class basic_image {
  using pixel_list = std::vector<PixelT>;

public:
  using pixel_type            = PixelT;
  using pixel_iterator        = typename pixel_list::iterator;
  using const_pixel_iterator  = typename pixel_list::const_iterator;
  using index                 = std::size_t;

  /// \brief Create an image of given dimensions.
  /// \throws std::logic_error Dimensions are invalid.
  basic_image(index width, index height)
    : pixels_(width * height)
    , width_{width}
  {
    if (width == 0 || height == 0)
      throw std::logic_error{"basic_image: Cannot construct a zero-size image"};
  }

  /// \name Observers
  ///@{
  index width() const noexcept  { return width_; }
  index height() const noexcept { return pixels_.size() / width(); }
  index size() const noexcept   { return pixels_.size(); }
  ///@}

  /// \name Iterator pixel access
  ///@{
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
  ///@}

  /// \name Random pixel access
  ///@{

  /// \brief Get pixel at given coordinates.
  /// \throws std::logic_error Coordinates out of bounds
  pixel_type&
  pixel_at(index x, index y);
  pixel_type const&
  pixel_at(index x, index y) const;

  ///@}

private:
  pixel_list  pixels_;
  std::size_t width_;
};

/// \name basic_image typedefs
///@{

/// \ingroup image

/// Channels in \f$[0, \infty)\f$.
using hdr_image = basic_image<hdr_color>;

/// Channels in \f$\{0, \ldots, 255\}\f$.
using ldr_image = basic_image<ldr_color>;

///@}

/// \name basic_image operations
///@{

/// \ingroup image

/// \brief Get the log-average luminance of a picture.
///
/// Log-average luminance of a picture is defined as
/// \f[
///   L_{\text{avg}}
///   = \exp\left(\frac{1}{N} \sum_{x,y}\log(\delta + L(x, y))\right)
/// \text{,}
/// \f]
/// where
/// - \f$N\f$ is the total number of pixels
/// - \f$\delta\f$ is a small positive number to avoid trouble with cases where
///   \f$L(x, y) = 0\f$
/// - \f$L(x, y)\f$ is the luminance of the pixel at \f$(x,y)\f$.
///
/// \f$L_{\text{avg}}\f$ is the geometric mean of luminances.
double
log_avg_luminance(hdr_image const& image);

/// \brief HDR → LDR transform.
///
/// Input pixels in range \f$[0, 1]\f$ are linearly mapped and rounded to byte
/// pixel values \f$\{0, \ldots, 255\}\f$. Input pixels > 1 are mapped to output
/// 255, producing a burn-out.
ldr_image
ldr_from_hdr(hdr_image const& hdr);

/// \brief Simulate real-life film exposure.
///
/// This transform image via
/// \f[
///   I_\mathrm{out}
///     = 1 - \exp\left(-\mathrm{exposure} \cdot I_\mathrm{in}\right)
///   \text{,}
/// \f]
/// where exposure is a positive parameter roughly corresponding to the
/// exposition time.
///
/// \todo This should throw when exposure is non-positive.
hdr_image
expose(hdr_image image, double exposure);

/// \brief Apply the Reinhard's operator.
///
/// Each channel is transformed by
/// \f{align*}{
///   I' &= I_\mathrm{in} \cdot \frac{\mathrm{key}}{L_\mathrm{avg}} \\ 
///   I_\mathrm{out} &= \frac{I'}{1 + I'} \text{,}
/// \f}
/// where key is a positive parameter and \f$L_\mathrm{avg}\f$ is log-average
/// luminance as returned by \ref log_avg_luminance.
///
/// \todo This should throw when key is non-positive.
hdr_image
apply_reinhard(hdr_image image, double key = 0.18);

/// \brief Perform gamma correction.
hdr_image
correct_gamma(hdr_image image, double gamma = 2.2);

/// \brief Save an LDR image into a PPM file.
/// \throws std::ios_base::failure I/O error.
void
save(ldr_image const& image, std::string const& filename);

///@}

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
