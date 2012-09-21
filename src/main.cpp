#include "image.hpp"

#include <algorithm>
#include <iostream>

auto main(int argc, char** argv) -> int {
  if (argc == 2) {
    char const* filename{argv[1]};

    oxatrace::image i{640, 480};
    std::fill(i.begin(), i.end(), oxatrace::color{0.5, 0.5, 0.5});
    oxatrace::save(i, filename);
  } else std::cerr << "Expected a filename, sorry.\n";
}

// vim:textwidth=80:colorcolumn=80
