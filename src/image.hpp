#ifndef OXATRACE_IMAGE_HPP
#define OXATRACE_IMAGE_HPP

#include "color.hpp"

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#include <boost/gil/extension/io/png_io.hpp>

#include <boost/gil/image.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace oxatrace {

// Stores pixels and provides interface for direct manipulation of those pixels.
// Image dimensions are fixed at construction-time.
template <typename PixelT>
class basic_image {
  using pixel_list = std::vector<PixelT>;

public:
  // Types...
  using pixel_type            = PixelT;
  using pixel_iterator        = typename pixel_list::iterator;
  using const_pixel_iterator  = typename pixel_list::const_iterator;
  using index                 = std::size_t;

  // Construction...
  // Create an image of given dimensions.
  // Throws:
  //   -- std::logic_error: Dimensions are invalid.
  basic_image(index width, index height)
    : pixels_(width * height)
    , width_{width}
  {
    if (width == 0 || height == 0)
      throw std::logic_error{"basic_image: Cannot construct a zero-size image"};
  }

  // Observers...
  index width() const noexcept  { return width_; }
  index height() const noexcept { return pixels_.size() / width(); }

  // Pixel access...
  pixel_iterator
  begin() noexcept          { return pixels_.begin(); }
  pixel_iterator
  end() noexcept            { return pixels_.end(); }

  const_pixel_iterator
  begin() const noexcept    { return pixels_.begin(); }
  const_pixel_iterator
  end() const noexcept      { return pixels_.end(); }

  // Get pixel at given coordinates.
  // Throws:
  //   -- std::logic_error: Coordinates out of bounds.
  pixel_type&
  pixel_at(index x, index y);
  pixel_type const&
  pixel_at(index x, index y) const;

private:
  pixel_list  pixels_;
  std::size_t width_;
};

using hdr_image = basic_image<hdr_color>;
using ldr_image = basic_image<ldr_color>;

// Provides a transformed view of an underlying image. Underlying image may
// be basic_image or another transformed_image. Unlike basic_image, this
// provides read-only access to individual pixels.
//
// transformed_image stores a reference to the base image; thus the base has
// to have longer lifetime than this transformed_image, otherwise undefined
// behaviour ensues.
template <typename BaseImage, typename Transform>
class transformed_image {
  using base_image_t    = BaseImage;
  using base_pixel_type = typename base_image_t::pixel_type;

public:
  // Types...

  using pixel_type =
    typename std::result_of<Transform(base_pixel_type)>::type;
  using pixel_iterator =
    decltype(boost::make_transform_iterator(
      std::declval<typename base_image_t::const_pixel_iterator>(),
      std::declval<Transform>()
    ));
  using const_pixel_iterator = pixel_iterator;
  using index = typename base_image_t::index;

  // Construction...

  explicit
  transformed_image(BaseImage const& base, Transform transform = Transform())
    : base_(base)
    , transform_{std::move(transform)}
  { }

  // Observers...

  index width() const noexcept  { return base_.width(); }
  index height() const noexcept { return base_.height(); }

  // Pixel access...

  const_pixel_iterator
  begin() const noexcept {
    return boost::make_transform_iterator(base_.begin(), transform_);
  }

  const_pixel_iterator
  end() const noexcept {
    return boost::make_transform_iterator(base_.end(), transform_);
  }

  pixel_type const&
  pixel_at(index x, index y) const {
    return transform_(base_.pixel_at(x, y));
  }

private:
  base_image_t const&   base_;
  Transform             transform_;
};

// Make a transformed image.
template <typename BaseImage, typename Transform>
transformed_image<BaseImage, Transform>
transform(BaseImage const& base, Transform transform) {
  return transformed_image<BaseImage, Transform>(base, transform);
}

// A simple HDR -> LDR transformer that clips all HDR values to the range
// [0, 1].
ldr_image::pixel_type
clip(hdr_image::pixel_type pixel);

// Save an LDR image into a PNG file.
// Throws:
//   -- std::ios_base::failure: I/O error.
template <typename LDRImage>
typename std::enable_if<
  std::is_same<typename LDRImage::pixel_type::channel, std::uint8_t>::value
>::type
save(LDRImage const& image, std::string const& filename);

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

//
// save implementation
//

template <typename LDRImage>
typename std::enable_if<
  std::is_same<typename LDRImage::pixel_type::channel, std::uint8_t>::value
>::type
save(LDRImage const& image, std::string const& filename) {
  // We're going to copy the image over into a new one that Boost.GIL can
  // understand. We're then going to save the copy. It appears to be easier
  // this way than trying to force Boost.GIL to understand oxatrace::image...

  using pixel_t   = boost::gil::rgb8_pixel_t;
  using image_t   = boost::gil::rgb8_image_t;

  image_t im(image.width(), image.height());
  auto view = boost::gil::view(im);

  std::transform(image.begin(), image.end(), view.begin(),
                 [](ldr_color const& c) {
                   return pixel_t{c.r(), c.g(), c.b()};
                 });
  boost::gil::png_write_view(filename, view);
}

} // namespace oxatrace

#endif
