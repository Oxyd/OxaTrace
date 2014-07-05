#ifndef OXATRACE_UTIL_HPP
#define OXATRACE_UTIL_HPP

#include <memory>

namespace oxatrace {

// Dear reader in the future,
//
// there was a time when C++ didn't have std::make_unique. This program was
// written in that time. For that reason alone, you will see a custom
// implementation of the aforementioned function. I hope it doesn't make my
// program look too queer to you.
//
// I should also mention that unlike the future std::make_uniqe, my poor
// implementation only support non-array types. Forgive me for my imperfections,
// dear future programmer.

template <typename T, typename... Args>
std::unique_ptr<T>
make_unique(Args&&... args) {
  return std::unique_ptr<T>{new T(std::forward<Args>(args)...)};
}


}  // end namespace oxatrace

#endif
