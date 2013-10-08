#ifndef OXATRACE_TEXT_INTERFACE_HPP
#define OXATRACE_TEXT_INTERFACE_HPP

#include <string>

#include <boost/optional.hpp>

namespace oxatrace {

/// \defgroup interface User interface

/// \brief Thingy to allow bits and pieces of code to report their progress.
/// \ingroup interface
///
/// Calculation progress is done in phases; the user is notified about each
/// new phase of calculation. A phase may optionally display its progress;
/// phase change resets progress back to zero.
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
