#ifndef OXATRACE_TEXT_INTERFACE_HPP
#define OXATRACE_TEXT_INTERFACE_HPP

#include <string>

#include <boost/optional.hpp>

namespace oxatrace {

class progress_monitor {
public:
  void
  change_phase(std::string const& new_phase);

  void
  update_progress(double progress);

private:
  static constexpr unsigned WIDTH = 40;
  static constexpr unsigned PRECISION = 2;

  bool                      progressbar_active_ = false;
  boost::optional<double>   last_progress_;
};

}

#endif
