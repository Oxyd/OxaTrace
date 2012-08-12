#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL

#include "math.hpp"


#include <boost/gil/image.hpp>
#include <boost/gil/pixel.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/png_io.hpp>

#include <iostream>
#include <algorithm>

auto main(int argc, char** argv) -> int {
  if (argc == 2) {
    char const* filename{argv[1]};

    using image_t = boost::gil::rgb8_image_t;
    using pixel_t = boost::gil::rgb8_pixel_t;

    image_t image{800, 600};

    auto view = boost::gil::view(image);
    std::fill(view.begin(), view.end(), pixel_t(60, 60, 255));

    boost::gil::png_write_view(filename, view);
  
  } else std::cerr << "Expected a filename, sorry.\n";
}

// vim:textwidth=80:colorcolumn=80
