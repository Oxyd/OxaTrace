#include "text_interface.hpp"

#include "math.hpp"

#include <iomanip>
#include <iostream>

using namespace oxatrace;

void
progress_monitor::change_phase(std::string const& new_phase) {
  if (progressbar_active_)
    std::cout << '\n';
  std::cout << new_phase << '\n';
  progressbar_active_ = false;
  last_progress_ = boost::none;
}

void
progress_monitor::update_progress(double progress) {
  constexpr unsigned DIGITS = 3 + 1 + PRECISION;
  constexpr double LEAST_INCREMENT = (1.0 / (10.0 * PRECISION)) / 100.0;

  if (progress < 0.0 || progress > 1.0)
    throw std::invalid_argument{"progress_monitor: progress outside [0, 1]"};

  if (!last_progress_ ||
      std::abs(progress - *last_progress_) >= LEAST_INCREMENT ||
      double_eq(progress, 1.0)) {
    std::cout << '\r'
              << std::fixed << std::setprecision(PRECISION)
              << std::setw(DIGITS) << std::setfill(' ')
              << progress * 100 << "% [";

    unsigned const prog = round<unsigned>(progress * WIDTH);
    for (unsigned i = 0; i < prog; ++i) std::cout << '#';
    for (unsigned i = prog; i < WIDTH; ++i) std::cout << ' ';
    std::cout << ']' << std::flush;

    last_progress_ = progress;
    progressbar_active_ = true;
  }
}
